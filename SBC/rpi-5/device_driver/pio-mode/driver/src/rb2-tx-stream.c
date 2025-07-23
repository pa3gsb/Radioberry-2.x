/*
Radioberry 2 TX stream handling using RP1 PIO

Transmits TX IQ samples from a KFIFO buffer over the FPGA FIFO.
*/

#include <linux/errno.h>
#include <linux/types.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/printk.h>

#include "rb2-rp1-pio.h"
#include "rb2-tx-stream.h"
#include "rb2-rpi5.h"


#define TX_SAMPLE_READY_PIN 12
#define TX_STREAM_IQ_DATA	5
#define TX_CLOCK_PIN        4		

#define NUM_TX_PINS 3
static const unsigned int tx_pio_pins[NUM_TX_PINS] = {TX_SAMPLE_READY_PIN, TX_STREAM_IQ_DATA, TX_CLOCK_PIN};

#define TX_SAMPLE_SIZE 			4096 		

const uint16_t tx_iq_sample_program_instructions[];
const struct pio_program tx_iq_sample_program;

void tx_iq_data_dma_callback(void *param);
void tx_dma_restart_work(struct work_struct *w);


static int radioberry_init_tx_ctx(struct radioberry_client_ctx *ctx)
{
    INIT_KFIFO(ctx->tx.dma_fifo);
    spin_lock_init(&ctx->tx.fifo_lock);
    init_waitqueue_head(&ctx->tx.queue);

    ctx->tx.dma_size = TX_SAMPLE_SIZE;
    ctx->tx.active_buffer = 0;

    init_completion(&ctx->tx.dma_done[0]);
    init_completion(&ctx->tx.dma_done[1]);
    complete(&ctx->tx.dma_done[0]);
    complete(&ctx->tx.dma_done[1]);
	
	atomic_set(&ctx->tx.dma_running, 0);

    INIT_WORK(&ctx->tx.dma_restart, tx_dma_restart_work);

    return 0;
}

int configure_tx_iq_sm(struct radioberry_client_ctx *ctx)
{
    initialize_gpio_for_input(TX_SAMPLE_READY_PIN);
    initialize_gpio_for_output(TX_CLOCK_PIN);
	clr_pin(TX_CLOCK_PIN);
    initialize_gpio_for_output(TX_STREAM_IQ_DATA);
	clr_pin(TX_STREAM_IQ_DATA);

    ctx->tx.client = rp1_pio_open();
    if (IS_ERR(ctx->tx.client)) {
        long err = PTR_ERR(ctx->tx.client);
        pr_err("radioberry: Failed to open TX PIO client\n");
        kfree(ctx);
        return err;
    }
	pr_info("RP1 PIO client for TX opened\n");

    int ret = radioberry_init_tx_ctx(ctx);
    if (ret) {
        pr_err("radioberry: Failed to initialize TX context: %d\n", ret);
        kfree(ctx);
        return ret;
    }

    struct rp1_pio_sm_claim_args args = {0};
    ctx->tx.sm = rp1_pio_sm_claim(ctx->tx.client, &args);
    if (ctx->tx.sm < 0) {
        pr_err("radioberry: Failed to claim TX PIO state machine\n");
        return ctx->tx.sm;
    }
	pr_info("radioberry: Claimed SM %d on PIO0 for TX\n", ctx->tx.sm);
	
	struct rp1_pio_sm_set_enabled_args notenable_args = {
		.mask = BIT(ctx->tx.sm),
		.enable = false,
	};
	ret = rp1_pio_sm_set_enabled(ctx->tx.client, &notenable_args);
	if (ret < 0) {
		pr_err("radioberry: Failed to disable TX SM %d\n", ctx->tx.sm);
	}
	pr_info("radioberry: SM %d disabled for TX\n", ctx->tx.sm);
	
	
    ret = rp1_pio_sm_config_xfer(ctx->tx.client, ctx->tx.sm, PIO_DIR_TO_SM, TX_SAMPLE_SIZE, 2);
    if (ret < 0) {
        pr_err("radioberry: Failed to configure TX DMA %d\n", ret);
        return ret;
    }
	pr_info("radioberry: Radioberry DMA rp1-pio configured for TX\n");

    struct rp1_pio_add_program_args prog_args;
    prog_args.num_instrs = tx_iq_sample_program.length;
    prog_args.origin = RP1_PIO_ORIGIN_ANY;
    ctx->tx.prog_length = tx_iq_sample_program.length;
    memcpy(prog_args.instrs, tx_iq_sample_program_instructions, ctx->tx.prog_length * sizeof(uint16_t));
	
	pr_info("radioberry: #TX-instructions: %zu\n", ctx->tx.prog_length);

    ctx->tx.prog_offset = rp1_pio_add_program(ctx->tx.client, &prog_args);
    if (ctx->tx.prog_offset < 0) {
        pr_err("radioberry: Failed to add TX PIO program\n");
        goto error_cleanup;
    }
	
	pr_info("radioberry: TX Program added at offset %d\n", ctx->tx.prog_offset);

    struct rp1_gpio_set_function_args init_pio_pin;
    for (int i = 0; i < NUM_TX_PINS; i++) {
        memset(&init_pio_pin, 0, sizeof(init_pio_pin));
        init_pio_pin.gpio = tx_pio_pins[i];
        init_pio_pin.fn = RP1_GPIO_FUNC_PIO;
        ret = rp1_pio_gpio_set_function(ctx->tx.client, &init_pio_pin);
        if (ret < 0) {
            pr_err("radioberry: Failed to init TX GPIO %d for PIO\n", tx_pio_pins[i]);
            goto error_cleanup;
        }
    }

    struct rp1_pio_sm_set_pindirs_args sample_ready_args = {
			.sm = ctx->tx.sm, .dirs = 0, .mask = BIT(TX_SAMPLE_READY_PIN),
    };
    ret = rp1_pio_sm_set_pindirs(ctx->tx.client, &sample_ready_args);
    if (ret < 0) {
        pr_err("radioberry: Failed to set TX SAMPLE_READY as output\n");
        goto error_cleanup;
    }
	
	struct rp1_pio_sm_set_pindirs_args sample_data_args = {
			.sm = ctx->tx.sm, .dirs = BIT(TX_STREAM_IQ_DATA), .mask = BIT(TX_STREAM_IQ_DATA),
    };
    ret = rp1_pio_sm_set_pindirs(ctx->tx.client, &sample_data_args);
    if (ret < 0) {
        pr_err("radioberry: Failed to set TX SAMPLE_DATA as output\n");
        goto error_cleanup;
    }

    struct rp1_pio_sm_set_pindirs_args clock_args = {
        .sm = ctx->tx.sm, .dirs = BIT(TX_CLOCK_PIN), .mask = BIT(TX_CLOCK_PIN),
    };
    ret = rp1_pio_sm_set_pindirs(ctx->tx.client, &clock_args);
    if (ret < 0) {
        pr_err("radioberry: Failed to set TX CLOCK as input\n");
        goto error_cleanup;
    }
	
	struct rp1_pio_sm_set_pins_args  set_pin_args = {
			.sm = ctx->tx.sm, .values = 0, .mask = BIT(TX_CLOCK_PIN),        
	};
	ret = rp1_pio_sm_set_pins(ctx->tx.client, &set_pin_args);
	if (ret < 0) {
		pr_err("radioberry: Failed to clear TX_CLOCK output\n");
		goto error_cleanup;
	}
	
	pr_info("radioberry: GPIO directions configured for TX SM %d\n", ctx->tx.sm);

    struct rp1_pio_sm_init_args config_args;
    config_args.sm = ctx->tx.sm;
    config_args.initial_pc = ctx->tx.prog_offset;
    config_args.config.clkdiv 		= 0x00080000;        
    config_args.config.execctrl 	= 0x4c01fb80; 
    config_args.config.shiftctrl 	= 0x00000000;  
    config_args.config.pinctrl		= 0x40101005; 
	
    ret = rp1_pio_sm_init(ctx->tx.client, &config_args);
    if (ret) {
        pr_err("radioberry: Failed to configure TX SM\n");
        goto error_cleanup;
    }
	pr_info("radioberry: SM %d configured for TX \n", ctx->tx.sm);

	struct rp1_pio_sm_clear_fifos_args clear_fifos_args = {.sm = ctx->tx.sm};
	ret = rp1_pio_sm_drain_tx(ctx->tx.client, &clear_fifos_args);
	if (ret < 0) {
		pr_err("radioberry: Failed to clear TX fifo SM %d\n", ctx->tx.sm);
		goto error_cleanup;
	}

    struct rp1_pio_sm_set_enabled_args enable_args = {
        .mask = BIT(ctx->tx.sm), .enable = true,
    };
    ret = rp1_pio_sm_set_enabled(ctx->tx.client, &enable_args);
    if (ret < 0) {
        pr_err("radioberry: Failed to enable TX SM %d\n", ctx->tx.sm);
        goto error_cleanup;
    }
	pr_info("radioberry: TX SM %d enabled\n", ctx->tx.sm);

    return 0;

error_cleanup:
    radioberry_cleanup_tx_ctx(ctx);
    return ret;
}

ssize_t rb2_tx_stream_write(struct radioberry_stream *tx, const uint8_t *data, size_t len)
{
    ssize_t written;
    unsigned long flags;
    bool should_schedule = false;
	
    spin_lock_irqsave(&tx->fifo_lock, flags);
		written = kfifo_in(&tx->dma_fifo, data, len);
		if (!atomic_read(&tx->dma_running) && kfifo_len(&tx->dma_fifo) >= tx->dma_size)	should_schedule = true;
    
	spin_unlock_irqrestore(&tx->fifo_lock, flags);

    if (should_schedule) schedule_work(&tx->dma_restart);

    return written;
}


void tx_dma_restart_work(struct work_struct *w)
{
    struct radioberry_stream *tx = container_of(w, struct radioberry_stream, dma_restart);
	
    size_t avail;
    unsigned long flags;

    spin_lock_irqsave(&tx->fifo_lock, flags);
		avail = kfifo_len(&tx->dma_fifo);
    spin_unlock_irqrestore(&tx->fifo_lock, flags);

    if (avail < tx->dma_size) {
        atomic_set(&tx->dma_running, 0);
        return;
    }
	if (atomic_xchg(&tx->dma_running, 1)) {
		pr_info("tx_dma_restart_work: DMA already running, skip");
		return;
	}
    int proc_buf = tx->active_buffer;
    uint8_t *iq_buf = tx->irq_scratch;

    spin_lock_irqsave(&tx->fifo_lock, flags);
		size_t actual = kfifo_out(&tx->dma_fifo, iq_buf, tx->dma_size);
    spin_unlock_irqrestore(&tx->fifo_lock, flags);

    uint32_t *tx_words = rp1_pio_sm_buffer_virt(
        tx->client,
        tx->sm,
        PIO_DIR_TO_SM,
        proc_buf
    );
	if (!tx_words) {
		pr_err("radioberry: NULL pointer van rp1_pio_sm_buffer_virt in TX worker!\n");
		atomic_set(&tx->dma_running, 0);
		return;
	}
    for (size_t i = 0; i < (actual / sizeof(uint32_t)); i++) {
          tx_words[i] =  ((iq_buf[i * 4 + 3] << 24) |
						  (iq_buf[i * 4 + 2] << 16) |
						  (iq_buf[i * 4 + 1] <<  8) |
						  (iq_buf[i * 4 + 0] <<  0));
    }
    int ret = rp1_pio_sm_xfer_data(
        tx->client,
        tx->sm,
        PIO_DIR_TO_SM,
        actual,
        tx_words, 0,
        tx_iq_data_dma_callback,
        tx
    );
    if (ret < 0) {
        pr_err("radioberry: TX DMA restart failed: %d\n", ret);
        atomic_set(&tx->dma_running, 0);
    }
}

void tx_iq_data_dma_callback(void *param)
{
	struct radioberry_stream *tx = param;
	tx->active_buffer = (tx->active_buffer + 1) & 1;
	atomic_set(&tx->dma_running, 0);
	size_t avail;
	unsigned long flags;
	spin_lock_irqsave(&tx->fifo_lock, flags);
		avail = kfifo_len(&tx->dma_fifo);
	spin_unlock_irqrestore(&tx->fifo_lock, flags);
	if (avail >= tx->dma_size) schedule_work(&tx->dma_restart);
}

void radioberry_cleanup_tx_ctx(struct radioberry_client_ctx *ctx)
{
    if (ctx->tx.client && ctx->tx.sm >= 0) {
        cancel_work_sync(&ctx->tx.dma_restart);
		
		pr_info("radioberry: Releasing TX SM %d and removing program\n", ctx->tx.sm);

        struct rp1_pio_sm_set_enabled_args disable_args = {
            .mask = BIT(ctx->tx.sm),
            .enable = false,
        };
        int ret = rp1_pio_sm_set_enabled(ctx->tx.client, &disable_args);
        if (ret < 0)
            pr_warn("radioberry: Failed to disable TX SM %d\n", ctx->tx.sm);

        struct rp1_pio_remove_program_args remove_args = {
            .origin = ctx->tx.prog_offset,
            .num_instrs = ctx->tx.prog_length
        };
        ret = rp1_pio_remove_program(ctx->tx.client, &remove_args);
        if (ret < 0)
            pr_warn("radioberry: Failed to remove TX PIO program (offset %d, len %d)\n",
                    ctx->tx.prog_offset, ctx->tx.prog_length);

        struct rp1_pio_sm_claim_args release_args = {
            .mask = BIT(ctx->tx.sm),
        };
        ret = rp1_pio_sm_unclaim(ctx->tx.client, &release_args);
        if (ret < 0)
            pr_warn("radioberry: Failed to unclaim TX SM %d\n", ctx->tx.sm);

        rp1_pio_close(ctx->tx.client);
		
		pr_info("radioberry: SM %d free for other use.\n", ctx->tx.sm);
    }
}

const uint16_t tx_iq_sample_program_instructions[] = {
            //     .wrap_target
    0x0005, //  0: jmp    5                          
    0x6201, //  1: out    pins, 1                [2] 
    0xba42, //  2: nop                    side 1 [2] 
    0xb242, //  3: nop                    side 0 [2] 
    0x0041, //  4: jmp    x--, 1                     
    0xe03f, //  5: set    x, 31                      
    0x80a0, //  6: pull   block                      
    0xba42, //  7: nop                    side 1 [2] 
    0xb242, //  8: nop                    side 0 [2] 
    0x00c1, //  9: jmp    pin, 1                     
            //     .wrap
};

const struct pio_program tx_iq_sample_program = {
    .instructions = tx_iq_sample_program_instructions,
    .length = 10,
    .origin = -1,
};

//end of source