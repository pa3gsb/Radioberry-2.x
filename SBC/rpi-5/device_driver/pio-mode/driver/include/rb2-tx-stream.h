#ifndef __RB2_TX_STREAM_H__
#define __RB2_TX_STREAM_H__


typedef unsigned int uint;
#include <linux/pio_rp1.h> 


int configure_tx_iq_sm(struct radioberry_client_ctx *ctx);
ssize_t rb2_tx_stream_write(struct radioberry_stream *tx, const uint8_t *data, size_t len);
void radioberry_cleanup_tx_ctx(struct radioberry_client_ctx *ctx);

void *rp1_pio_sm_buffer_virt(struct rp1_pio_client *client, unsigned sm, unsigned dir, int index);

#endif