/*

Radioberry stream handling using the PIO mode of the RP1 IO chip.

Supports the receive stream process:

Input:
Reading RX IQ samples as present in the FPGA receive FIFO of the radioberry radio card.

Output:
The IQ samples are delivered in a KFIFO buffer and available for reading.

Process:
	- Loading PIO program
	- Setting up DMA process
	- Retrieve the IQ samples and fill the KFIFO.
	
	- Unloading the PIO program.
*/

#include <linux/errno.h>
#include <linux/types.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/printk.h>

#include "rb2-rp1-pio.h"
#include "rb2-rx-stream.h"
#include "rb2-rpi5.h"

#define RX_STREAM_IQ_CLK  	 6
#define RX_STREAM_IQ_RDY 	25
#define RX_STREAM_IQ_D3  	21
#define RX_STREAM_IQ_D2  	20
#define RX_STREAM_IQ_D1  	19
#define RX_STREAM_IQ_D0  	18

#define NUM_RX_DATA_PINS 4
static const unsigned int data_pins[NUM_RX_DATA_PINS] = {RX_STREAM_IQ_D0, RX_STREAM_IQ_D1, RX_STREAM_IQ_D2, RX_STREAM_IQ_D3};
#define RX_SAMPLE_READY_PIN RX_STREAM_IQ_RDY
#define RX_CLOCK_PIN        RX_STREAM_IQ_CLK

#define RX_SAMPLES 			159 	// [0-159] = 160 reads from FPGA FIFO => 80 IQ SAMPLES


const uint16_t rx_iq_sample_program_instructions[];
const struct pio_program rx_iq_sample_program;
void dma_restart_work(struct work_struct *w);
void rx_iq_data_dma_callback(void *param);



int radioberry_init_ctx(struct radioberry_client_ctx *ctx)
{
    INIT_KFIFO(ctx->rx.dma_fifo);
    spin_lock_init(&ctx->rx.fifo_lock);
    init_waitqueue_head(&ctx->rx.queue);

	ctx->rx.dma_size = SAMPLE_SIZE;// 24576; 
	ctx->rx.active_buffer = 0;

	init_completion(&ctx->rx.dma_done[0]);
	init_completion(&ctx->rx.dma_done[1]);

	complete(&ctx->rx.dma_done[0]); 
	complete(&ctx->rx.dma_done[1]);
	
	atomic_set(&ctx->rx.dma_running, 0); 
	
	INIT_WORK(&ctx->rx.dma_restart, dma_restart_work);
	
    return 0;
}

int configure_rx_iq_sm(struct radioberry_client_ctx *ctx)
{	
	initialize_gpio_for_output(RX_STREAM_IQ_CLK);
	clr_pin(RX_STREAM_IQ_CLK); 			
	initialize_gpio_for_input(RX_STREAM_IQ_RDY);		
	initialize_gpio_for_input(RX_STREAM_IQ_D3);	
	initialize_gpio_for_input(RX_STREAM_IQ_D2);	
	initialize_gpio_for_input(RX_STREAM_IQ_D1);	
	initialize_gpio_for_input(RX_STREAM_IQ_D0);	
		
	ctx->rx.client = rp1_pio_open();
	if (IS_ERR(ctx->rx.client)) {
		long err = PTR_ERR(ctx->rx.client); 
		pr_err("Failed to open PIO client\n");
		kfree(ctx);
		return err;
	}
	pr_info("RP1 PIO client for RX opened\n");
	
	
	int ret = radioberry_init_ctx(ctx);
	if (ret) {
		pr_err("Failed to initialize context: %d\n", ret);
		kfree(ctx);
		return ret;
	}	
	
	struct rp1_pio_sm_claim_args args = {0};
    args.mask = 0;
    ctx->rx.sm = rp1_pio_sm_claim(ctx->rx.client, &args);
    if (ctx->rx.sm < 0) {
		pr_err("radioberry: Failed to claim PIO state machine\n");
		return ctx->rx.sm;
    }
    pr_info("Claimed SM %d on PIO0 for RX\n", ctx->rx.sm);
	
	struct rp1_pio_sm_set_enabled_args notenable_args = {
		.mask = BIT(ctx->rx.sm),
		.enable = false,
	};
	ret = rp1_pio_sm_set_enabled(ctx->rx.client, &notenable_args);
	if (ret < 0) {
		pr_err("radioberry: Failed to disable SM %d\n", ctx->rx.sm);
	}
	pr_info("radioberry: SM %d disabled for RX\n", ctx->rx.sm);
	

	ret = rp1_pio_sm_config_xfer(ctx->rx.client, ctx->rx.sm, PIO_DIR_FROM_SM, SAMPLE_SIZE, 2);
	if (ret < 0) {
		pr_err("radioberry: Failed to configure DMA %d\n", ret);
		return ret;
	}
	pr_info("radioberry: Radioberry DMA rp1-pio configured for RX\n");
	
	struct rp1_pio_add_program_args prog_args;
    prog_args.num_instrs = rx_iq_sample_program.length;
    prog_args.origin = RP1_PIO_ORIGIN_ANY;
	ctx->rx.prog_length = rx_iq_sample_program.length;
	memcpy(prog_args.instrs, rx_iq_sample_program_instructions, ctx->rx.prog_length * sizeof(uint16_t));
	
	pr_info("radioberry: #RX-instructions: %zu\n", ctx->rx.prog_length);
	
    ctx->rx.prog_offset = rp1_pio_add_program(ctx->rx.client, &prog_args);
    if (ctx->rx.prog_offset < 0) {
        pr_err("radioberry: Failed to add PIO program\n");
        goto error_cleanup;
    }
    pr_info("radioberry: RX Program added at offset %d\n", ctx->rx.prog_offset);

	//initialize IO pins as PIO pins
	struct rp1_gpio_set_function_args init_pio_pin; 
	
	for (int i = 0; i < NUM_RX_DATA_PINS; i++) {
		memset(&init_pio_pin, 0, sizeof(init_pio_pin));
		init_pio_pin.gpio = data_pins[i];
		init_pio_pin.fn = RP1_GPIO_FUNC_PIO;
		ret = rp1_pio_gpio_set_function(ctx->rx.client, &init_pio_pin); 
		if (ret < 0) {
			pr_err("radioberry: Failed to init GPIO %d for PIO\n", data_pins[i]);
			goto error_cleanup;
		}
	}
	init_pio_pin.gpio = RX_SAMPLE_READY_PIN; 
	init_pio_pin.fn = RP1_GPIO_FUNC_PIO;
	ret = rp1_pio_gpio_set_function(ctx->rx.client, &init_pio_pin);
	if (ret < 0) {
		pr_err("radioberry: Failed to init RX_SAMPLE_READY_PIN for PIO\n");
		goto error_cleanup;
	}
	init_pio_pin.gpio = RX_CLOCK_PIN; 
	init_pio_pin.fn = RP1_GPIO_FUNC_PIO;
	ret = rp1_pio_gpio_set_function(ctx->rx.client, &init_pio_pin);
	if (ret < 0) {
		pr_err("radioberry: Failed to init RX_CLOCK_PIN for PIO\n");
		goto error_cleanup;
	}
	
    // Configureer GPIO richtingen
    for (int i = 0; i < NUM_RX_DATA_PINS; i++) {
        struct rp1_pio_sm_set_pindirs_args dir_args = {
            .sm = ctx->rx.sm, .dirs = 0, .mask = BIT(data_pins[i]),
        };
        ret = rp1_pio_sm_set_pindirs(ctx->rx.client, &dir_args);
        if (ret < 0) {
            pr_err("radioberry: Failed to set data GPIO %d as input\n", data_pins[i]);
            goto error_cleanup;
        }
    }

    struct rp1_pio_sm_set_pindirs_args sample_ready_args = {
			.sm = ctx->rx.sm, .dirs = 0, .mask = BIT(RX_SAMPLE_READY_PIN),
    };
    ret = rp1_pio_sm_set_pindirs(ctx->rx.client, &sample_ready_args);
    if (ret < 0) {
        pr_err("radioberry: Failed to set SAMPLE_READY as input\n");
        goto error_cleanup;
    }

    struct rp1_pio_sm_set_pindirs_args clock_args = {
        .sm = ctx->rx.sm, .dirs = BIT(RX_CLOCK_PIN), .mask = BIT(RX_CLOCK_PIN),
    };
    ret = rp1_pio_sm_set_pindirs(ctx->rx.client, &clock_args);
    if (ret < 0) {
        pr_err("radioberry: Failed to set RX_CLOCK as output\n");
        goto error_cleanup;
    }
    pr_info("radioberry: GPIO directions configured for RX SM %d\n", ctx->rx.sm);
	
	struct rp1_pio_sm_set_pins_args  set_pin_args = {
			.sm = ctx->rx.sm, .values = 0, .mask = BIT(RX_CLOCK_PIN),        
	};
	ret = rp1_pio_sm_set_pins(ctx->rx.client, &set_pin_args);
	if (ret < 0) {
		pr_err("radioberry: Failed to clear RX_CLOCK output\n");
		goto error_cleanup;
	}
	pr_info("radioberry: Pins set to output (rx clock) en input pins\n");
	
	// sm configuriation
	struct rp1_pio_sm_init_args config_args;
	config_args.sm = ctx->rx.sm;
	config_args.initial_pc = ctx->rx.prog_offset;
	config_args.config.clkdiv 		= 0x00020000;
	config_args.config.execctrl 	= 0x5901f680;
	config_args.config.shiftctrl 	= 0x00000000;
	config_args.config.pinctrl 		= 0x40091800;

	ret = rp1_pio_sm_init(ctx->rx.client, &config_args);
	if (ret) {
		pr_err("radioberry: Failed to configure SM\n");
		goto error_cleanup;
	}
	pr_info("radioberry: SM %d configured for RX \n", ctx->rx.sm);


	struct rp1_pio_sm_clear_fifos_args clear_fifos_args = {.sm = ctx->rx.sm};
	ret = rp1_pio_sm_drain_tx(ctx->rx.client, &clear_fifos_args);
	if (ret < 0) {
		pr_err("radioberry: Failed to clear RX fifo SM %d\n", ctx->rx.sm);
		goto error_cleanup;
	}
	
	struct rp1_pio_sm_set_enabled_args enable_args = {
			.mask = BIT(ctx->rx.sm), .enable = true,
	};
	ret = rp1_pio_sm_set_enabled(ctx->rx.client, &enable_args);
	if (ret < 0) {
		pr_err("radioberry: Failed to enable RX SM %d\n", ctx->rx.sm);
		goto error_cleanup;
	}
	pr_info("radioberry: RX SM %d enabled\n", ctx->rx.sm);
	

	struct rp1_pio_sm_put_args put_args = {
			.sm = ctx->rx.sm, .blocking = true, .data = RX_SAMPLES
	};
	ret = rp1_pio_sm_put(ctx->rx.client, &put_args);
	if (ret < 0) {
		pr_err("radioberry: Failed to put data to sm %d\n", ctx->rx.sm);
		goto error_cleanup;
	}
	pr_info("radioberry: Radioberry put #RX_SAMPLES in sm\n");	
	
	// Start RX DMA transfer
    schedule_work(&ctx->rx.dma_restart); 

	return 0;

error_cleanup:
    radioberry_cleanup_rx_ctx(ctx);
    return ret;	
}

void dma_restart_work(struct work_struct *w)
{
    struct radioberry_stream *rx = container_of(w, struct radioberry_stream, dma_restart);

	if (atomic_xchg(&rx->dma_running, 1)) {
        return;
    }

    int proc_buf = rx->active_buffer;

    void *dst = rp1_pio_sm_buffer_virt(
        rx->client,
        rx->sm,
        PIO_DIR_FROM_SM,
        proc_buf
    );
    if (!dst) {
		atomic_set(&rx->dma_running, 0);
        pr_err("radioberry: FROM_SM buffer_virt is NULL (buf=%d)\n", proc_buf);
        return;
    }

    int ret = rp1_pio_sm_xfer_data(
        rx->client,
        rx->sm,
        PIO_DIR_FROM_SM,
        rx->dma_size,
        dst,                  
        0,
        rx_iq_data_dma_callback,
        rx
    );
    if (ret < 0){
		atomic_set(&rx->dma_running, 0);
        pr_err("radioberry: DMA-restart failed: %d\n", ret);
	}
}


void rx_iq_data_dma_callback(void *param)
{
	struct radioberry_stream *rx = param;

	int proc_buf = rx->active_buffer;
	uint8_t *iq_buf = rx->irq_scratch;
	size_t iq_count = 0;

    uint32_t *iq_words = rp1_pio_sm_buffer_virt(
        rx->client,
        rx->sm,
        PIO_DIR_FROM_SM,
        proc_buf
    );
	
	for (size_t i = 0; i < (rx->dma_size / sizeof(uint32_t)); i++) {
		iq_buf[iq_count++] = (iq_words[i] >> 16) & 0xFF;   
		iq_buf[iq_count++] = (iq_words[i] >> 8) & 0xFF;   
		iq_buf[iq_count++] = iq_words[i] & 0xFF; 
	}

	unsigned long flags;
	spin_lock_irqsave(&rx->fifo_lock, flags);
	{
	if (kfifo_avail(&rx->dma_fifo) < iq_count) kfifo_reset(&rx->dma_fifo);
	unsigned copied = kfifo_in(&rx->dma_fifo, iq_buf, iq_count);
	}
	spin_unlock_irqrestore(&rx->fifo_lock, flags);

    wake_up_interruptible(&rx->queue);
    rx->active_buffer = (proc_buf + 1) & 1;
	atomic_set(&rx->dma_running, 0);
	schedule_work(&rx->dma_restart);
}

void radioberry_cleanup_rx_ctx(struct radioberry_client_ctx *ctx)
{
	
	if (ctx->rx.client && ctx->rx.sm >= 0) { 
	
		cancel_work_sync(&ctx->rx.dma_restart);
		atomic_set(&ctx->rx.dma_running, 0);

		pr_info("Radioberry: Releasing RX SM %d and removing program\n", ctx->rx.sm);

		struct rp1_pio_sm_set_enabled_args disable_args = {
			.mask = BIT(ctx->rx.sm),
			.enable = false,
		};
		int ret = rp1_pio_sm_set_enabled(ctx->rx.client, &disable_args);
		if (ret < 0)
			pr_warn("radioberry: Failed to disable SM %d\n", ctx->rx.sm);
	
		struct rp1_pio_remove_program_args remove_args = {
			.origin = ctx->rx.prog_offset,
			.num_instrs = ctx->rx.prog_length
		};
		ret = rp1_pio_remove_program(ctx->rx.client, &remove_args);
		if (ret < 0)
			pr_warn("radioberry: Failed to remove PIO program (offset %d, len %d)\n",
					ctx->rx.prog_offset, ctx->rx.prog_length);

		struct rp1_pio_sm_claim_args release_args = {
			.mask = BIT(ctx->rx.sm),
		};
		ret = rp1_pio_sm_unclaim(ctx->rx.client, &release_args);
		if (ret < 0)
			pr_warn("radioberry: Failed to unclaim SM %d\n", ctx->rx.sm);

		rp1_pio_close(ctx->rx.client);
		
		pr_info("radioberry: SM %d free for other use.\n", ctx->rx.sm);
	}
}

const uint16_t rx_iq_sample_program_instructions[] = {
    0x80a0, //  0: pull   block
    0xa047, //  1: mov    y, osr
    0xa022, //  2: mov    x, y
            //     .wrap_target
    0x0013, //  3: jmp    19
    0xbb42, //  4: nop                    side 1 [3]
    0x5004, //  5: in     pins, 4         side 0
    0xbb42, //  6: nop                    side 1 [3]
    0x5004, //  7: in     pins, 4         side 0
    0xbb42, //  8: nop                    side 1 [3]
    0x5004, //  9: in     pins, 4         side 0
    0xbb42, // 10: nop                    side 1 [3]
    0x5004, // 11: in     pins, 4         side 0
    0xbb42, // 12: nop                    side 1 [3]
    0x5004, // 13: in     pins, 4         side 0
    0xbb42, // 14: nop                    side 1 [3]
    0x5004, // 15: in     pins, 4         side 0
    0x8020, // 16: push   block
    0x0044, // 17: jmp    x--, 4
    0xa022, // 18: mov    x, y
    0xba42, // 19: nop                    side 1 [2]
    0xb242, // 20: nop                    side 0 [2]
    0x00c4, // 21: jmp    pin, 4
            //     .wrap
};

const struct pio_program rx_iq_sample_program = {
    .instructions = rx_iq_sample_program_instructions,
    .length = 22,
    .origin = -1,
};

//end of source