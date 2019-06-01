#ifndef TINKERBOARD_IO_H
#define TINKERBOARD_IO_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include "tinkerboard_spi.h"


#define ALIGN(x)  ((x) / 4)
#define TO_INDEX(x)  ((x)-1)
#define VALID_GPIO(x)  ((x)>0 && (x)<=40 ? 1 : 0)
#define NO_POWER_PIN(x) ((x) != 1 && (x) != 2 && (x) != 4 && (x) != 6 && (x) != 9 && (x) != 14 && (x) != 17 \
                        && (x) != 20 && (x) != 25 && (x) != 30 && (x) != 34 && (x) != 39)

#define RK3288_GPIO_BLOCK_BASE    0xFF730000
#define RK3288_GPIO_BLOCK_SIZE    0xD0000

#define RK3288_GPIO0_OFFSET       0x20000
#define RK3288_GPIO5_OFFSET       0x90000
#define RK3288_GPIO6_OFFSET       0xA0000
#define RK3288_GPIO7_OFFSET       0xB0000
#define RK3288_GPIO8_OFFSET       0xC0000

#define RK3288_GPIO0C_GRF_OFFSET  0x0008C
#define RK3288_GPIO5B_GRF_OFFSET  0x40050
#define RK3288_GPIO5C_GRF_OFFSET  0x40054
#define RK3288_GPIO6A_GRF_OFFSET  0x4005C
#define RK3288_GPIO7A_GRF_OFFSET  0x4006C
#define RK3288_GPIO7B_GRF_OFFSET  0x40070
#define RK3288_GPIO7CL_GRF_OFFSET 0x40074
#define RK3288_GPIO7CH_GRF_OFFSET 0x40078
#define RK3288_GPIO8A_GRF_OFFSET  0x40080
#define RK3288_GPIO8B_GRF_OFFSET  0x40084

#define RK3288_GPIO0C_GRF_P_OFFSET 0x0006C
#define RK3288_GPIO5B_GRF_P_OFFSET 0x40184
#define RK3288_GPIO5C_GRF_P_OFFSET 0x40188
#define RK3288_GPIO6A_GRF_P_OFFSET 0x40190
#define RK3288_GPIO7A_GRF_P_OFFSET 0x401A0
#define RK3288_GPIO7B_GRF_P_OFFSET 0x401A4
#define RK3288_GPIO7C_GRF_P_OFFSET 0x401A8
#define RK3288_GPIO8A_GRF_P_OFFSET 0x401B0
#define RK3288_GPIO8B_GRF_P_OFFSET 0x401B4

#define RK3288_GPIO0C_GRF_E_OFFSET 0x00078
#define RK3288_GPIO5B_GRF_E_OFFSET 0x40204
#define RK3288_GPIO5C_GRF_E_OFFSET 0x40208
#define RK3288_GPIO6A_GRF_E_OFFSET 0x40210
#define RK3288_GPIO7A_GRF_E_OFFSET 0x40220
#define RK3288_GPIO7B_GRF_E_OFFSET 0x40224
#define RK3288_GPIO7C_GRF_E_OFFSET 0x40228
#define RK3288_GPIO8A_GRF_E_OFFSET 0x40230
#define RK3288_GPIO8B_GRF_E_OFFSET 0x40234

#define RK3288_SPI_BLOCK_BASE     0xFF110000
#define RK3288_SPI0_BLOCK_OFFSET  0x00000
#define RK3288_SPI1_BLOCK_OFFSET  0x10000
#define RK3288_SPI2_BLOCK_OFFSET  0x20000
#define RK3288_SPI_BLOCK_SIZE     0x30000

#define RK3288_CONFIG_WRITEMASK_OFFSET 16

#define RK3288_CRU_BLOCK_BASE	  0xFF760000
#define RK3288_CRU_BLOCK_SIZE	  0x10000

#define RK3288_CRU_SOFTRST_OFFSET 0x01CC
#define RK3288_CRU_CLKSEL39_OFFSET 0x00FC
#define RK3288_CRU_CLKSEL25_OFFSET 0x00C4
#define RK3288_CRU_CLKGATE2_OFFSET 0x0168
#define RK3288_CRU_CLKGATE6_OFFSET 0x0178

#ifdef __cplusplus
extern "C"{
#endif

//#define NO_SS 	0x0
enum SlaveSelect { NO_SS = 0x0, SS_0 = 0x1, SS_1 = 0x2, SS_2 = 0x3};
enum IOState {HIGH = 0x1, LOW = 0x0};
enum IOMode {INPUT = 0x0, OUTPUT = 0x1, SPI = 0x2, I2C = 0x3, PWM = 0x4};
enum SPIController {SPI0 = 0x0, SPI2 = 0x1};
//enum SPISlaveSelect {SS_NONE = 0x1, SS0 = 0x2, SS1 = 0x3};
enum SPIDataFrameSize {DFS_4 = 0x0, DFS_8 = 0x1, DFS_16 = 0x2};
enum SPIByteTransform {BT_16_8 = 0x0, BT_8_8 = 0x1};
enum SPITransferMode {TRANSMIT_RECEIVE = 0x0, TRANSMIT = 0x1, RECEIVE = 0x2};
enum SPIByteOrder {MSB_FIRST = 0x0, LSB_FIRST = 0x1};
enum PUDMode {NORMAL_Z = 0x0, PULLUP = 0x1, PULLDOWN = 0x2, BUSKEEPER = 0x3};
enum DriveStrength {DRV2MA = 0x0, DRV4MA = 0x1, DRV8MA = 0x2, DRV12MA = 0x3};


struct gpio_pin_t {
  uint32_t gpio_bank_offset;
  uint32_t gpio_control_offset;
  uint32_t grf_bank_offset;
  uint32_t grf_pin_offset;
  uint32_t grf_config_size;
  uint32_t grf_pud_offset;
  uint32_t grf_drvstrg_offset;
  uint32_t grf_pd_pin_offset;
  enum IOMode mode;
  uint32_t is_gpio;
};

struct spi_pin_config_t {
  uint32_t clk;
  uint32_t txd;
  uint32_t rxd;
  uint32_t cs0;
  uint32_t cs1;
  uint32_t cs2;
  uint32_t spi_block_offset;
  uint32_t pll_sel_offset;
  uint32_t clk_src_offset;
  uint32_t clk_gate_flag;
  uint32_t pclk_src_offset;
  uint32_t pclk_gate_flag;
  uint32_t softrst_offset;
  uint32_t softrst_flag;
  uint32_t initialized;
};

struct spi_mode_config_t {
  uint32_t clk_divider;
  uint32_t clk_mode;
  enum SPIDataFrameSize data_frame_size;
  uint32_t slave_select_0;
  uint32_t slave_select_1;
  uint32_t slave_select_2;
  enum SPITransferMode transfer_mode;
  enum SPIByteOrder  byte_order;
};

uint32_t tinkerboard_get_grf_config(uint32_t pin_number);

void tinkerboard_set_gpio_mode(uint32_t pin_number, enum IOMode mode);

enum IOMode tinkerboard_get_gpio_mode(uint32_t pin_number);

void tinkerboard_set_gpio_state(uint32_t pin_number, enum IOState state);

enum IOState tinkerboard_get_gpio_state(uint32_t pin_number);

void tinkerboard_set_gpio_pud(uint32_t pin_number, enum PUDMode mode);

void tinkerboard_set_gpio_drive_strength(uint32_t pin_number, enum DriveStrength strength);

void tinkerboard_reset_header(void);

int tinkerboard_init(void);

void tinkerboard_end(void);

void tinkerboard_spi_init(enum SPIController controller, struct spi_mode_config_t mode_config);

void tinkerboard_spi_end(enum SPIController controller);

void tinkerboard_spi_transfer(enum SPIController controller, uint8_t* tx_buff, uint8_t* rx_buff, uint32_t length, struct spi_mode_config_t mode_config, enum SlaveSelect slave);

#ifdef __cplusplus
}
#endif

#endif
