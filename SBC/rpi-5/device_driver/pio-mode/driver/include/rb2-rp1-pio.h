#ifndef __RB2_RP1_PIO_H__
#define __RB2_RP1_PIO_H__


#include <linux/types.h>
#include <linux/completion.h>
#include <linux/workqueue.h>
#include <linux/kfifo.h>
#include <linux/spinlock.h>
#include <linux/wait.h>

#define RP1_GPIO_FUNC_PIO	7

#define SAMPLE_SIZE 24576
#define FIFO_SIZE 	32768  

	struct radioberry_stream {
		
		//PIO
		struct rp1_pio_client *client;
		int sm;
		int prog_offset;
		int prog_length;
		
		//DMA
		size_t dma_size;
		int active_buffer;
		struct completion dma_done[2];
		struct work_struct dma_restart;
		atomic_t dma_running; 							// 0 = idle, 1 = busy
		
		//FIFO
		DECLARE_KFIFO(dma_fifo, uint8_t, FIFO_SIZE);
		wait_queue_head_t queue;
		spinlock_t fifo_lock;
		
		//one buffer.
		uint8_t irq_scratch[SAMPLE_SIZE];
	};

	struct radioberry_client_ctx {
		struct radioberry_stream rx;
		struct radioberry_stream tx;
	};
	
	
#endif
