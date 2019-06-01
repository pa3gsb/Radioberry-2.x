#ifndef TINKERBOARD_SPI_H
#define TINKERBOARD_SPI_H


#define ROCKCHIP_SPI_CTRLR0    0x0000
#define ROCKCHIP_SPI_CTRLR1    0x0004
#define ROCKCHIP_SPI_ENR       0x0008
#define ROCKCHIP_SPI_SER       0x000C
#define ROCKCHIP_SPI_BAUDR     0x0010
#define ROCKCHIP_SPI_TXFTLR    0x0014
#define ROCKCHIP_SPI_RXFTLR    0x0018
#define ROCKCHIP_SPI_TXFLR     0x001C
#define ROCKCHIP_SPI_RXFLR     0x0020
#define ROCKCHIP_SPI_SR        0x0024
#define ROCKCHIP_SPI_IPR       0x0028
#define ROCKCHIP_SPI_IMR       0x002C
#define ROCKCHIP_SPI_ISR       0x0030
#define ROCKCHIP_SPI_RISR      0x0034
#define ROCKCHIP_SPI_ICR       0x0038
#define ROCKCHIP_SPI_DMACR     0x003C
#define ROCKCHIP_SPI_DMATDLR   0x0040
#define ROCKCHIP_SPI_DMARDLR   0x0044
#define ROCKCHIP_SPI_TXDR      0x0400
#define ROCKCHIP_SPI_RXDR      0x0800

#define CR0_DFS_OFFSET         0
#define CR0_CFS_OFFSET         2

#define CR0_SCPH_OFFSET        6
#define CR0_SCPOL_OFFSET       7

#define CR0_CSM_OFFSET         8
#define CR0_CSM_KEEP           0x0
#define CR0_CSM_HALF           0X1
#define CR0_CSM_ONE            0x2

#define CR0_SSD_OFFSET         10
#define CR0_SSD_HALF           0x0
#define CR0_SSD_ONE            0x1

#define CR0_EM_OFFSET          11
#define CR0_EM_LITTLE          0x0
#define CR0_EM_BIG             0x1

#define CR0_FBM_OFFSET         12
#define CR0_FBM_MSB            0x0
#define CR0_FBM_LSB            0x1

#define CR0_BHT_OFFSET         13
#define CR0_BHT_16BIT          0x0
#define CR0_BHT_8BIT           0x1

#define CR0_RSD_OFFSET         14

#define CR0_FRF_OFFSET         16
#define CR0_FRF_SPI            0x0
#define CR0_FRF_SSP            0x1
#define CR0_FRF_MICROWIRE      0x2

#define CR0_XFM_OFFSET         18
#define CR0_XFM_MASK           (0x03 << 18)
#define CR0_XFM_TR             0x0
#define CR0_XFM_TO             0x1
#define CR0_XFM_RO             0x2

#define CR0_OPM_OFFSET         20
#define CR0_OPM_MASTER         0x0
#define CR0_OPM_SLAVE          0x1

#define CR0_MTM_OFFSET         0x21

#define SER_MASK               0x3

#define SR_MASK                0x1F
#define SR_BUSY                (1 << 0)
#define SR_TF_FULL             (1 << 1)
#define SR_TF_EMPTY            (1 << 2)
#define SR_RF_EMPTY            (1 << 3)
#define SR_RF_FULL             (1 << 4)

#define INT_MASK               0x1F
#define INT_TF_EMPTY           (1 << 0)
#define INT_TF_OVERFLOW        (1 << 1)
#define INT_RF_UNDERFLOW       (1 << 2)
#define INT_RF_OVERFLOW        (1 << 3)
#define INT_RF_FULL            (1 << 4)

#define ICR_MASK               0x0F
#define ICR_ALL                (1 << 0)
#define ICR_RF_UNDERFLOW       (1 << 1)
#define ICR_RF_OVERFLOW        (1 << 2)
#define ICR_TF_OVERFLOW        (1 << 3)

#define RF_DMA_EN              (1 << 0)
#define TF_DMA_EN              (1 << 1)

#define RXBUSY                 (1 << 0)
#define TXBUSY                 (1 << 1)

#define MAX_SCLK_OUT           50000000
#define SPI_WAIT_FOR_IDLE_MICROS 50000

#ifdef __cplusplus
extern "C"{
#endif

struct spi_internals_t {
  uint32_t fifo_len;
  uint8_t* rx;
  uint8_t* tx;
  uint8_t* rx_end;
  uint8_t* tx_end;
  uint32_t cs0_pin;
  uint32_t cs1_pin;
  uint32_t cs2_pin;
};

#ifdef __cplusplus
}
#endif

#endif
