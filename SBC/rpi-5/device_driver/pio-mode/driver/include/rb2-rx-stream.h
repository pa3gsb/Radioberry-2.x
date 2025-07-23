#ifndef __RB2_RX_STREAM_H__
#define __RB2_RX_STREAM_H__

typedef unsigned int uint;
#include <linux/pio_rp1.h> 


struct radioberry_client_ctx;
int configure_rx_iq_sm(struct radioberry_client_ctx *ctx);
void radioberry_cleanup_rx_ctx(struct radioberry_client_ctx *ctx);

void *rp1_pio_sm_buffer_virt(struct rp1_pio_client *client, unsigned sm, unsigned dir, int index);

#endif
