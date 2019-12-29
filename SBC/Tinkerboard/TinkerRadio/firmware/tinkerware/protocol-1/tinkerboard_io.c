#include "tinkerboard_io.h"

uint32_t *_rk3288_cru_block_base = (uint32_t *) RK3288_CRU_BLOCK_BASE;
uint32_t _rk3288_cru_block_size = RK3288_CRU_BLOCK_SIZE;

uint32_t *_rk3288_gpio_block_base = (uint32_t *) RK3288_GPIO_BLOCK_BASE;
uint32_t _rk3288_gpio_block_size = RK3288_GPIO_BLOCK_SIZE;

uint32_t *_rk3288_spi_block_base = (uint32_t *) RK3288_SPI_BLOCK_BASE;
uint32_t _rk3288_spi_block_size = RK3288_SPI_BLOCK_SIZE;

struct gpio_pin_t _gpio_header_pins[] = {
    {.gpio_bank_offset = 0, .gpio_control_offset = 0, .grf_bank_offset = 0, .grf_pin_offset = 0, .grf_config_size = 0, .is_gpio = 0, .mode = INPUT}, //1
    {.gpio_bank_offset = 0, .gpio_control_offset = 0, .grf_bank_offset = 0, .grf_pin_offset = 0, .grf_config_size = 0, .is_gpio = 0, .mode = INPUT}, //2
    {.gpio_bank_offset = RK3288_GPIO8_OFFSET, .gpio_control_offset =  4, .grf_bank_offset = RK3288_GPIO8A_GRF_OFFSET, .grf_pin_offset =  8, //3
        .grf_config_size = 2, .grf_pud_offset = RK3288_GPIO8A_GRF_P_OFFSET, .grf_drvstrg_offset = RK3288_GPIO8A_GRF_E_OFFSET, .grf_pd_pin_offset = 8,
        .is_gpio = 1, .mode = INPUT},
    {.gpio_bank_offset = 0, .gpio_control_offset = 0, .grf_bank_offset = 0, .grf_pin_offset = 0, .grf_config_size = 0, .is_gpio = 0, .mode = INPUT}, //4
    {.gpio_bank_offset = RK3288_GPIO8_OFFSET, .gpio_control_offset =  5, .grf_bank_offset = RK3288_GPIO8A_GRF_OFFSET, .grf_pin_offset = 10, //5
        .grf_config_size = 2, .grf_pud_offset = RK3288_GPIO8A_GRF_P_OFFSET, .grf_drvstrg_offset = RK3288_GPIO8A_GRF_E_OFFSET, .grf_pd_pin_offset = 10,
        .is_gpio = 1, .mode = INPUT},
    {.gpio_bank_offset = 0, .gpio_control_offset = 0, .grf_bank_offset = 0, .grf_pin_offset = 0, .grf_config_size = 0, .is_gpio = 0, .mode = INPUT}, //6
    {.gpio_bank_offset = RK3288_GPIO0_OFFSET, .gpio_control_offset = 17, .grf_bank_offset = RK3288_GPIO0C_GRF_OFFSET, .grf_pin_offset = 2, //7
        .grf_config_size = 2, .grf_pud_offset = RK3288_GPIO0C_GRF_P_OFFSET, .grf_drvstrg_offset = RK3288_GPIO0C_GRF_E_OFFSET, .grf_pd_pin_offset = 2,
        .is_gpio = 1, .mode = INPUT},
    {.gpio_bank_offset = RK3288_GPIO5_OFFSET, .gpio_control_offset =  9, .grf_bank_offset = RK3288_GPIO5B_GRF_OFFSET, .grf_pin_offset =  2, //8
        .grf_config_size = 2, .grf_pud_offset = RK3288_GPIO5B_GRF_P_OFFSET, .grf_drvstrg_offset = RK3288_GPIO5B_GRF_E_OFFSET, .grf_pd_pin_offset = 2,
        .is_gpio = 1, .mode = INPUT},

    {.gpio_bank_offset = 0, .gpio_control_offset = 0, .grf_bank_offset = 0, .grf_pin_offset = 0, .grf_config_size = 0, .is_gpio = 0, .mode = INPUT}, //9
    {.gpio_bank_offset = RK3288_GPIO5_OFFSET, .gpio_control_offset =  8, .grf_bank_offset = RK3288_GPIO5B_GRF_OFFSET, .grf_pin_offset =  0, //10
        .grf_config_size = 2, .grf_pud_offset = RK3288_GPIO5B_GRF_P_OFFSET, .grf_drvstrg_offset = RK3288_GPIO5B_GRF_E_OFFSET, .grf_pd_pin_offset = 0,
        .is_gpio = 1, .mode = INPUT},
    {.gpio_bank_offset = RK3288_GPIO5_OFFSET, .gpio_control_offset = 12, .grf_bank_offset = RK3288_GPIO5B_GRF_OFFSET, .grf_pin_offset =  8, //11
        .grf_config_size = 2, .grf_pud_offset = RK3288_GPIO5B_GRF_P_OFFSET, .grf_drvstrg_offset = RK3288_GPIO5B_GRF_E_OFFSET, .grf_pd_pin_offset = 8,
        .is_gpio = 1, .mode = INPUT},
    {.gpio_bank_offset = RK3288_GPIO6_OFFSET, .gpio_control_offset =  0, .grf_bank_offset = RK3288_GPIO6A_GRF_OFFSET, .grf_pin_offset =  0, //12
        .grf_config_size = 1, .grf_pud_offset = RK3288_GPIO6A_GRF_P_OFFSET, .grf_drvstrg_offset = RK3288_GPIO6A_GRF_E_OFFSET, .grf_pd_pin_offset = 0,
        .is_gpio = 1, .mode = INPUT},
    {.gpio_bank_offset = RK3288_GPIO5_OFFSET, .gpio_control_offset = 14, .grf_bank_offset = RK3288_GPIO5B_GRF_OFFSET, .grf_pin_offset = 12, //13
        .grf_config_size = 2, .grf_pud_offset = RK3288_GPIO5B_GRF_P_OFFSET, .grf_drvstrg_offset = RK3288_GPIO5B_GRF_E_OFFSET, .grf_pd_pin_offset = 12,
        .is_gpio = 1, .mode = INPUT},
    {.gpio_bank_offset = 0, .gpio_control_offset = 0, .grf_bank_offset = 0, .grf_pin_offset = 0, .grf_config_size = 0, .is_gpio = 0, .mode = INPUT}, //14
    {.gpio_bank_offset = RK3288_GPIO5_OFFSET, .gpio_control_offset = 15, .grf_bank_offset = RK3288_GPIO5B_GRF_OFFSET, .grf_pin_offset = 14, //15
        .grf_config_size = 2, .grf_pud_offset = RK3288_GPIO5B_GRF_P_OFFSET, .grf_drvstrg_offset = RK3288_GPIO5B_GRF_E_OFFSET, .grf_pd_pin_offset = 14,
        .is_gpio = 1, .mode = INPUT},
    {.gpio_bank_offset = RK3288_GPIO5_OFFSET, .gpio_control_offset = 10, .grf_bank_offset = RK3288_GPIO5B_GRF_OFFSET, .grf_pin_offset =  4, //16
        .grf_config_size = 2, .grf_pud_offset = RK3288_GPIO5B_GRF_P_OFFSET, .grf_drvstrg_offset = RK3288_GPIO5B_GRF_E_OFFSET, .grf_pd_pin_offset = 4,
        .is_gpio = 1, .mode = INPUT},

    {.gpio_bank_offset = 0, .gpio_control_offset = 0, .grf_bank_offset = 0, .grf_pin_offset = 0, .grf_config_size = 0, .is_gpio = 0, .mode = INPUT}, //17
    {.gpio_bank_offset = RK3288_GPIO5_OFFSET, .gpio_control_offset = 11, .grf_bank_offset = RK3288_GPIO5B_GRF_OFFSET, .grf_pin_offset =  6, //18
        .grf_config_size = 2, .grf_pud_offset = RK3288_GPIO5B_GRF_P_OFFSET, .grf_drvstrg_offset = RK3288_GPIO5B_GRF_E_OFFSET, .grf_pd_pin_offset = 6,
        .is_gpio = 1, .mode = INPUT},
    {.gpio_bank_offset = RK3288_GPIO8_OFFSET, .gpio_control_offset =  9, .grf_bank_offset = RK3288_GPIO8B_GRF_OFFSET, .grf_pin_offset =  2, //19
        .grf_config_size = 2, .grf_pud_offset = RK3288_GPIO8B_GRF_P_OFFSET, .grf_drvstrg_offset = RK3288_GPIO8B_GRF_E_OFFSET, .grf_pd_pin_offset = 2,
        .is_gpio = 1, .mode = INPUT},
    {.gpio_bank_offset = 0, .gpio_control_offset = 0, .grf_bank_offset = 0, .grf_pin_offset = 0, .grf_config_size = 0, .is_gpio = 0, .mode = INPUT}, //20
    {.gpio_bank_offset = RK3288_GPIO8_OFFSET, .gpio_control_offset =  8, .grf_bank_offset = RK3288_GPIO8B_GRF_OFFSET, .grf_pin_offset =  0, //21
        .grf_config_size = 2, .grf_pud_offset = RK3288_GPIO8B_GRF_P_OFFSET, .grf_drvstrg_offset = RK3288_GPIO8B_GRF_E_OFFSET, .grf_pd_pin_offset = 0,
        .is_gpio = 1, .mode = INPUT},
    {.gpio_bank_offset = RK3288_GPIO5_OFFSET, .gpio_control_offset = 19, .grf_bank_offset = RK3288_GPIO5C_GRF_OFFSET, .grf_pin_offset =  6, //22
        .grf_config_size = 1, .grf_pud_offset = RK3288_GPIO5C_GRF_P_OFFSET, .grf_drvstrg_offset = RK3288_GPIO5C_GRF_E_OFFSET, .grf_pd_pin_offset = 6,
        .is_gpio = 1, .mode = INPUT},
    {.gpio_bank_offset = RK3288_GPIO8_OFFSET, .gpio_control_offset =  6, .grf_bank_offset = RK3288_GPIO8A_GRF_OFFSET, .grf_pin_offset = 12, //23
        .grf_config_size = 2, .grf_pud_offset = RK3288_GPIO8A_GRF_P_OFFSET, .grf_drvstrg_offset = RK3288_GPIO8A_GRF_E_OFFSET, .grf_pd_pin_offset = 12,
        .is_gpio = 1, .mode = INPUT},
    {.gpio_bank_offset = RK3288_GPIO8_OFFSET, .gpio_control_offset =  7, .grf_bank_offset = RK3288_GPIO8A_GRF_OFFSET, .grf_pin_offset = 14, //24
        .grf_config_size = 2, .grf_pud_offset = RK3288_GPIO8A_GRF_P_OFFSET, .grf_drvstrg_offset = RK3288_GPIO8A_GRF_E_OFFSET, .grf_pd_pin_offset = 14,
        .is_gpio = 1, .mode = INPUT},

    {.gpio_bank_offset = 0, .gpio_control_offset = 0, .grf_bank_offset = 0, .grf_pin_offset = 0, .grf_config_size = 0, .is_gpio = 0, .mode = INPUT}, //25
    {.gpio_bank_offset = RK3288_GPIO8_OFFSET, .gpio_control_offset =  3, .grf_bank_offset = RK3288_GPIO8A_GRF_OFFSET, .grf_pin_offset =  6, //26
        .grf_config_size = 2, .grf_pud_offset = RK3288_GPIO8A_GRF_P_OFFSET, .grf_drvstrg_offset = RK3288_GPIO8A_GRF_E_OFFSET, .grf_pd_pin_offset = 6,
        .is_gpio = 1, .mode = INPUT},
    {.gpio_bank_offset = RK3288_GPIO7_OFFSET, .gpio_control_offset = 17, .grf_bank_offset = RK3288_GPIO7CL_GRF_OFFSET, .grf_pin_offset = 4, //27
        .grf_config_size = 1, .grf_pud_offset = RK3288_GPIO7C_GRF_P_OFFSET, .grf_drvstrg_offset = RK3288_GPIO7C_GRF_E_OFFSET, .grf_pd_pin_offset = 2,
        .is_gpio = 1, .mode = INPUT},
    {.gpio_bank_offset = RK3288_GPIO7_OFFSET, .gpio_control_offset = 18, .grf_bank_offset = RK3288_GPIO7CL_GRF_OFFSET, .grf_pin_offset = 8, //28
        .grf_config_size = 1, .grf_pud_offset = RK3288_GPIO7C_GRF_P_OFFSET, .grf_drvstrg_offset = RK3288_GPIO7C_GRF_E_OFFSET, .grf_pd_pin_offset = 4,
        .is_gpio = 1, .mode = INPUT},
    {.gpio_bank_offset = RK3288_GPIO5_OFFSET, .gpio_control_offset = 13, .grf_bank_offset = RK3288_GPIO5B_GRF_OFFSET, .grf_pin_offset = 10, //29
        .grf_config_size = 2, .grf_pud_offset = RK3288_GPIO5B_GRF_P_OFFSET, .grf_drvstrg_offset = RK3288_GPIO5B_GRF_E_OFFSET, .grf_pd_pin_offset = 10,
        .is_gpio = 1, .mode = INPUT},
    {.gpio_bank_offset = 0, .gpio_control_offset = 0, .grf_bank_offset = 0, .grf_pin_offset = 0, .grf_config_size = 0, .is_gpio = 0, .mode = INPUT}, //30
    {.gpio_bank_offset = RK3288_GPIO5_OFFSET, .gpio_control_offset = 16, .grf_bank_offset = RK3288_GPIO5C_GRF_OFFSET, .grf_pin_offset =  0, //31
        .grf_config_size = 2, .grf_pud_offset = RK3288_GPIO5C_GRF_P_OFFSET, .grf_drvstrg_offset = RK3288_GPIO5C_GRF_E_OFFSET, .grf_pd_pin_offset = 0,
        .is_gpio = 1, .mode = INPUT},
    {.gpio_bank_offset = RK3288_GPIO7_OFFSET, .gpio_control_offset = 23, .grf_bank_offset = RK3288_GPIO7CH_GRF_OFFSET, .grf_pin_offset = 12, //32
        .grf_config_size = 3, .grf_pud_offset = RK3288_GPIO7C_GRF_P_OFFSET, .grf_drvstrg_offset = RK3288_GPIO7C_GRF_E_OFFSET, .grf_pd_pin_offset = 14,
        .is_gpio = 1, .mode = INPUT},

    {.gpio_bank_offset = RK3288_GPIO7_OFFSET, .gpio_control_offset = 22, .grf_bank_offset = RK3288_GPIO7CH_GRF_OFFSET, .grf_pin_offset = 8, //33
        .grf_config_size = 2, .grf_pud_offset = RK3288_GPIO7C_GRF_P_OFFSET, .grf_drvstrg_offset = RK3288_GPIO7C_GRF_E_OFFSET, .grf_pd_pin_offset = 12,
        .is_gpio = 1, .mode = INPUT},
    {.gpio_bank_offset = 0, .gpio_control_offset = 0, .grf_bank_offset = 0, .grf_pin_offset = 0, .grf_config_size = 0, .is_gpio = 0,.mode = INPUT}, //34
    {.gpio_bank_offset = RK3288_GPIO6_OFFSET, .gpio_control_offset =  1, .grf_bank_offset = RK3288_GPIO6A_GRF_OFFSET, .grf_pin_offset =  2, //35
        .grf_config_size = 1, .grf_pud_offset = RK3288_GPIO6A_GRF_P_OFFSET, .grf_drvstrg_offset = RK3288_GPIO6A_GRF_E_OFFSET, .grf_pd_pin_offset = 2,
        .is_gpio = 1, .mode = INPUT},
    {.gpio_bank_offset = RK3288_GPIO7_OFFSET, .gpio_control_offset =  7, .grf_bank_offset = RK3288_GPIO7A_GRF_OFFSET, .grf_pin_offset = 14, //36
        .grf_config_size = 2, .grf_pud_offset = RK3288_GPIO7A_GRF_P_OFFSET, .grf_drvstrg_offset = RK3288_GPIO7A_GRF_E_OFFSET, .grf_pd_pin_offset = 14,
        .is_gpio = 1, .mode = INPUT},
    {.gpio_bank_offset = RK3288_GPIO7_OFFSET, .gpio_control_offset =  8, .grf_bank_offset = RK3288_GPIO7B_GRF_OFFSET, .grf_pin_offset =  0, //37
        .grf_config_size = 2, .grf_pud_offset = RK3288_GPIO7B_GRF_P_OFFSET, .grf_drvstrg_offset = RK3288_GPIO7B_GRF_E_OFFSET, .grf_pd_pin_offset = 0,
        .is_gpio = 1, .mode = INPUT},
    {.gpio_bank_offset = RK3288_GPIO6_OFFSET, .gpio_control_offset =  3, .grf_bank_offset = RK3288_GPIO6A_GRF_OFFSET, .grf_pin_offset =  6, //38
        .grf_config_size = 1, .grf_pud_offset = RK3288_GPIO6A_GRF_P_OFFSET, .grf_drvstrg_offset = RK3288_GPIO6A_GRF_E_OFFSET, .grf_pd_pin_offset = 6,
        .is_gpio = 1, .mode = INPUT},
    {.gpio_bank_offset = 0, .gpio_control_offset = 0, .grf_bank_offset = 0, .grf_pin_offset = 0, .grf_config_size = 0, .is_gpio = 0, .mode = INPUT}, //39
    {.gpio_bank_offset = RK3288_GPIO6_OFFSET, .gpio_control_offset =  4, .grf_bank_offset = RK3288_GPIO6A_GRF_OFFSET, .grf_pin_offset =  8, //40
        .grf_config_size = 1, .grf_pud_offset = RK3288_GPIO6A_GRF_P_OFFSET, .grf_drvstrg_offset = RK3288_GPIO6A_GRF_E_OFFSET, .grf_pd_pin_offset = 8,
        .is_gpio = 1, .mode = INPUT}
};

struct spi_pin_config_t _spi_configs[] = {
	{.clk = 10, .txd = 12, .rxd = 14, .cs0 = 28, .cs1 = 30, .spi_block_offset = RK3288_SPI0_BLOCK_OFFSET, .initialized = 0,
		.pll_sel_offset = RK3288_CRU_CLKSEL25_OFFSET,
		.clk_src_offset = RK3288_CRU_CLKGATE2_OFFSET, .clk_gate_flag = 9,
		.pclk_src_offset = RK3288_CRU_CLKGATE6_OFFSET, .pclk_gate_flag = 4,
		.softrst_offset = RK3288_CRU_SOFTRST_OFFSET, .softrst_flag = 3},

	{.clk = 22, .txd = 18, .rxd = 20, .cs0 = 23, .cs1 = 25, .spi_block_offset = RK3288_SPI2_BLOCK_OFFSET, .initialized = 0,
		.pll_sel_offset = RK3288_CRU_CLKSEL39_OFFSET,
		.clk_src_offset = RK3288_CRU_CLKGATE2_OFFSET, .clk_gate_flag = 11,
		.pclk_src_offset = RK3288_CRU_CLKGATE6_OFFSET, .pclk_gate_flag = 6,
		.softrst_offset = RK3288_CRU_SOFTRST_OFFSET, .softrst_flag = 5},
};

struct spi_internals_t _spi_internals[] = {
    {.fifo_len = 0, .tx = 0, .rx = 0, .rx_end = 0, .tx_end = 0, .cs0_pin = 0, .cs1_pin = 0, .cs2_pin = 0},
    {.fifo_len = 0, .tx = 0, .rx = 0, .rx_end = 0, .tx_end = 0, .cs0_pin = 0, .cs1_pin = 0, .cs2_pin = 0}
};

/*
*
* HELPER FUNCTIONS
*
*/

static inline int64_t _timediff_micro(struct timespec first, struct timespec second) {
  return (((second.tv_sec - first.tv_sec) * 1000000) + (second.tv_nsec - first.tv_nsec)) / 1000;
}

static inline uint32_t _min(uint32_t first, uint32_t second) {
  return first <= second ? first : second;
}

static inline void _write_mem(volatile uint32_t *mem_addr, uint32_t value) {
  __sync_synchronize();
  *mem_addr = value;
  __sync_synchronize();
}

static inline uint32_t _read_mem(volatile uint32_t *mem_addr) {
  __sync_synchronize();
  uint32_t ret = 0;
  ret = *mem_addr;
  __sync_synchronize();
  return ret;
}

static inline void _set_bit(uint32_t *value, uint32_t bit) {
  *value |= 1 << bit;
}

static inline void _clear_bit(uint32_t *value, uint32_t bit) {
  *value &= ~(1 << bit);
}

static inline uint32_t _generate_bitmask(uint32_t start, uint32_t size) {
  uint32_t ret = 0;
  for (uint32_t idx = start; idx <= start + size - 1; idx++) {
    ret |= 1 << idx;
  }
  return ret;
}

static inline void _set_config(uint32_t *register_addr, uint32_t offset, uint32_t config, uint32_t config_length) {
  uint32_t register_data = _read_mem(register_addr);

  for (uint32_t idx = 0; idx < config_length; idx++) {
    if(config & (1 << idx)) {
      _set_bit(&register_data, offset + idx);
    } else {
      _clear_bit(&register_data, offset + idx);
    }
    _set_bit(&register_data, offset + idx + RK3288_CONFIG_WRITEMASK_OFFSET);
  }

  _write_mem(register_addr, register_data);
}

/*
*
* GPIO FUNCTIONS
*/

uint32_t tinkerboard_get_grf_config(uint32_t pin_number) {
  if (VALID_GPIO(pin_number) && _gpio_header_pins[TO_INDEX(pin_number)].is_gpio) {
    uint32_t register_data = _read_mem(_rk3288_gpio_block_base + ALIGN(_gpio_header_pins[TO_INDEX(pin_number)].grf_bank_offset));
    return (_generate_bitmask(_gpio_header_pins[TO_INDEX(pin_number)].grf_pin_offset,
                              _gpio_header_pins[TO_INDEX(pin_number)].grf_config_size) & register_data)
                              >> _gpio_header_pins[TO_INDEX(pin_number)].grf_pin_offset;
  }
  return 0;
}

void tinkerboard_set_gpio_pud(uint32_t pin_number, enum PUDMode mode) {
  if (VALID_GPIO(pin_number) && _gpio_header_pins[TO_INDEX(pin_number)].is_gpio && pin_number) {
    _set_config(_rk3288_gpio_block_base + ALIGN(_gpio_header_pins[TO_INDEX(pin_number)].grf_pud_offset),
                _gpio_header_pins[TO_INDEX(pin_number)].grf_pd_pin_offset, mode, 2);
  }
}

void tinkerboard_set_gpio_drive_strength(uint32_t pin_number, enum DriveStrength strength) {
  if (VALID_GPIO(pin_number) && _gpio_header_pins[TO_INDEX(pin_number)].is_gpio && pin_number) {
    _set_config(_rk3288_gpio_block_base + ALIGN(_gpio_header_pins[TO_INDEX(pin_number)].grf_drvstrg_offset),
                _gpio_header_pins[TO_INDEX(pin_number)].grf_pd_pin_offset, strength, 2);
  }
}

void tinkerboard_set_gpio_mode(uint32_t pin_number, enum IOMode mode) {
  if (VALID_GPIO(pin_number) && _gpio_header_pins[TO_INDEX(pin_number)].is_gpio) {

    // Set IOMUX of requested pin to gpio
    _set_config(_rk3288_gpio_block_base + ALIGN(_gpio_header_pins[TO_INDEX(pin_number)].grf_bank_offset),
                _gpio_header_pins[TO_INDEX(pin_number)].grf_pin_offset, 0, _gpio_header_pins[TO_INDEX(pin_number)].grf_config_size);


    // Set value of gpio in "data out" register to LOW. If gpio becomes an output it will be LOW initially
   // uint32_t register_data = _read_mem(_rk3288_gpio_block_base + ALIGN(_gpio_header_pins[TO_INDEX(pin_number)].gpio_bank_offset));
   // _clear_bit(&register_data, _gpio_header_pins[TO_INDEX(pin_number)].gpio_control_offset);
  //  _write_mem(_rk3288_gpio_block_base + ALIGN(_gpio_header_pins[TO_INDEX(pin_number)].gpio_bank_offset), register_data);

    // Set gpio to either INPUT or OUTPUT
   uint32_t  register_data = _read_mem(_rk3288_gpio_block_base + ALIGN(_gpio_header_pins[TO_INDEX(pin_number)].gpio_bank_offset) + 0x01);
    if (mode == INPUT) {
      _clear_bit(&register_data, _gpio_header_pins[TO_INDEX(pin_number)].gpio_control_offset);
      _gpio_header_pins[TO_INDEX(pin_number)].mode = INPUT;
    } else if (mode == OUTPUT) {
      _set_bit(&register_data, _gpio_header_pins[TO_INDEX(pin_number)].gpio_control_offset);
      _gpio_header_pins[TO_INDEX(pin_number)].mode = OUTPUT;
    }
    _write_mem(_rk3288_gpio_block_base + ALIGN(_gpio_header_pins[TO_INDEX(pin_number)].gpio_bank_offset) + 0x01, register_data);
  }
}

void tinkerboard_set_gpio_state(uint32_t pin_number, enum IOState state) {
  if (VALID_GPIO(pin_number) && _gpio_header_pins[TO_INDEX(pin_number)].is_gpio && _gpio_header_pins[TO_INDEX(pin_number)].mode == OUTPUT) {

    uint32_t register_data = _read_mem(_rk3288_gpio_block_base + ALIGN(_gpio_header_pins[TO_INDEX(pin_number)].gpio_bank_offset));

    if (state == LOW) {
      _clear_bit(&register_data, _gpio_header_pins[TO_INDEX(pin_number)].gpio_control_offset);
    } else if (state == HIGH) {
      _set_bit(&register_data, _gpio_header_pins[TO_INDEX(pin_number)].gpio_control_offset);
    }

    _write_mem(_rk3288_gpio_block_base + ALIGN(_gpio_header_pins[TO_INDEX(pin_number)].gpio_bank_offset), register_data);
  }
}

enum IOState tinkerboard_get_gpio_state(uint32_t pin_number) {
  if (VALID_GPIO(pin_number) && _gpio_header_pins[TO_INDEX(pin_number)].is_gpio && _gpio_header_pins[TO_INDEX(pin_number)].mode == INPUT) {

    // Read input register and get bit of the requested pin
    uint32_t register_data = _read_mem(_rk3288_gpio_block_base + ALIGN(_gpio_header_pins[TO_INDEX(pin_number)].gpio_bank_offset + 0x50));
    uint32_t state = (register_data >> _gpio_header_pins[TO_INDEX(pin_number)].gpio_control_offset) & 0x1;

    if (state) {
      return HIGH;
    } else {
      return LOW;
    }
  }

  return LOW;
}

enum IOMode tinkerboard_get_gpio_mode(uint32_t pin_number){
  if (VALID_GPIO(pin_number) && _gpio_header_pins[TO_INDEX(pin_number)].is_gpio) {
    uint32_t register_data = _read_mem(_rk3288_gpio_block_base + ALIGN(_gpio_header_pins[TO_INDEX(pin_number)].gpio_bank_offset) + 0x01);
    uint32_t  state = (register_data >> _gpio_header_pins[TO_INDEX(pin_number)].gpio_control_offset) & 0x1;

    if (state) {
      return OUTPUT;
    } else {
      return INPUT;
    }
  }

  return INPUT;
}

/*
*
* SETUP FUNCTIONS
*
*/

void tinkerboard_reset_header(void) {
  for (uint32_t idx = 1; idx <= 40; idx++) {
    tinkerboard_set_gpio_mode(idx, INPUT);
    tinkerboard_set_gpio_pud(idx, PULLUP);
  }
}

int tinkerboard_init(void) {
  int memfd = -1;
  int retcode = 0;

  if ((memfd = open("/dev/mem", O_RDWR | O_SYNC)) < 0) {
    printf("Couldn't open /dev/mem: %s\n", strerror(errno));
    goto end;
  }

  _rk3288_cru_block_base = mmap(NULL, _rk3288_cru_block_size, (PROT_READ | PROT_WRITE), MAP_SHARED, memfd, RK3288_CRU_BLOCK_BASE);
  if (_rk3288_cru_block_base == MAP_FAILED) {
    printf("Error while mapping cru block into virtual memory\n");
    goto end;
  } else {
	//printf("Mapped CRU block to: %p\n", _rk3288_cru_block_base);
  }

  _rk3288_gpio_block_base = mmap(NULL, _rk3288_gpio_block_size, (PROT_READ | PROT_WRITE), MAP_SHARED, memfd, RK3288_GPIO_BLOCK_BASE);
  if (_rk3288_gpio_block_base == MAP_FAILED) {
    printf("Error while mapping gpio block into virtual memory\n");
    goto end;
  } else {
	//printf("Mapped GPIO block to: %p\n", _rk3288_gpio_block_base);
  }

  _rk3288_spi_block_base = mmap(NULL, _rk3288_spi_block_size, (PROT_READ | PROT_WRITE), MAP_SHARED, memfd, RK3288_SPI_BLOCK_BASE);
  if (_rk3288_spi_block_base == MAP_FAILED) {
    printf("Error while mapping spi block into virtual memory\n");
    goto end;
  } else {
	//printf("Mapped SPI block to: %p\n", _rk3288_spi_block_base);
  }

  retcode = 1;

  end:
  if (memfd >= 0) {
    close(memfd);
  }

  if (retcode == 1) {
    //tinkerboard_reset_header();
  }

  return retcode;
}

void tinkerboard_end(void) {
  if (_rk3288_gpio_block_base != MAP_FAILED) {
    //tinkerboard_reset_header();
    munmap((void **) _rk3288_gpio_block_base, _rk3288_gpio_block_size);
    _rk3288_gpio_block_base = MAP_FAILED;
  }


  if (_rk3288_spi_block_base != MAP_FAILED) {
    munmap((void **) _rk3288_spi_block_base, _rk3288_spi_block_size);
    _rk3288_spi_block_base = MAP_FAILED;
  }


  if (_rk3288_cru_block_base != MAP_FAILED) {
    munmap((void **) _rk3288_cru_block_base, _rk3288_cru_block_size);
    _rk3288_cru_block_base = MAP_FAILED;
  }
}



/*
*
* SPI FUNCTIONS
*
*/

static inline void _spi_enable_controller(enum SPIController controller, uint32_t config) {
  _write_mem(_rk3288_spi_block_base + ALIGN(_spi_configs[controller].spi_block_offset + ROCKCHIP_SPI_ENR), config);
//  printf("SPI Enable %08X at %p\n", _read_mem(_rk3288_spi_block_base + ALIGN(_spi_configs[controller].spi_block_offset + ROCKCHIP_SPI_ENR)), _rk3288_spi_block_base + ALIGN(_spi_configs[controller].spi_block_offset + ROCKCHIP_SPI_ENR));
}

static inline void _spi_set_slave_select(enum SPIController controller) {
  _write_mem(_rk3288_spi_block_base + ALIGN(_spi_configs[controller].spi_block_offset + ROCKCHIP_SPI_SER), 0x1);
//  printf("SPI SS %08X at %p\n", _read_mem(_rk3288_spi_block_base + ALIGN(_spi_configs[controller].spi_block_offset + ROCKCHIP_SPI_SER)), _rk3288_spi_block_base + ALIGN(_spi_configs[controller].spi_block_offset + ROCKCHIP_SPI_SER));
}

static inline void _spi_set_fifo_size(enum SPIController controller, uint32_t fifo_size) {
  _write_mem(_rk3288_spi_block_base + ALIGN(_spi_configs[controller].spi_block_offset + ROCKCHIP_SPI_TXFTLR), fifo_size);
  _write_mem(_rk3288_spi_block_base + ALIGN(_spi_configs[controller].spi_block_offset + ROCKCHIP_SPI_RXFTLR), fifo_size);
}

static inline void _spi_set_clk_divider(enum SPIController controller, uint32_t divider) {
  if(divider >= 2 && divider <= 65534) {
    _write_mem(_rk3288_spi_block_base + ALIGN(_spi_configs[controller].spi_block_offset + ROCKCHIP_SPI_BAUDR), divider);
  } else {
	_write_mem(_rk3288_spi_block_base + ALIGN(_spi_configs[controller].spi_block_offset + ROCKCHIP_SPI_BAUDR), 2);
  }
}

static inline void _spi_set_ctrlr0(enum SPIController controller, uint32_t config) {
  _write_mem(_rk3288_spi_block_base + ALIGN(_spi_configs[controller].spi_block_offset + ROCKCHIP_SPI_CTRLR0), config);
//  printf("SPI Ctrl0 %08X at %p\n", _read_mem(_rk3288_spi_block_base + ALIGN(_spi_configs[controller].spi_block_offset + ROCKCHIP_SPI_CTRLR0)), _rk3288_spi_block_base + ALIGN(_spi_configs[controller].spi_block_offset + ROCKCHIP_SPI_CTRLR0));
}

static inline void _spi_wait_for_idle(enum SPIController controller) {
  struct timespec start, now;

  clock_gettime(CLOCK_MONOTONIC_RAW, &start);
  do {
    if(!(_read_mem(_rk3288_spi_block_base + ALIGN(_spi_configs[controller].spi_block_offset + ROCKCHIP_SPI_SR)) & SR_BUSY)) {
      return;
    }
    clock_gettime(CLOCK_MONOTONIC_RAW, &now);
  } while(_timediff_micro(start, now) < SPI_WAIT_FOR_IDLE_MICROS);
  printf("SPI timed out\n");
}

static inline void _spi_send(enum SPIController controller, struct spi_mode_config_t mode_config){
    uint32_t tx_left, tx_room;

    tx_left = (_spi_internals[controller].tx_end - _spi_internals[controller].tx) / mode_config.data_frame_size;
    tx_room = _spi_internals[controller].fifo_len - _read_mem(_rk3288_spi_block_base +
                                                              ALIGN(_spi_configs[controller].spi_block_offset + ROCKCHIP_SPI_TXFLR));

    uint32_t max = _min(tx_left, tx_room);
    uint32_t txw = 0;

    while (max--) {
      if (mode_config.data_frame_size == DFS_8) {
        txw = *(uint8_t *)(_spi_internals[controller].tx);
      } else {
        txw = *(uint16_t * )(_spi_internals[controller].tx);
      }

      _write_mem(_rk3288_spi_block_base + ALIGN(_spi_configs[controller].spi_block_offset + ROCKCHIP_SPI_TXDR), txw);
      _spi_internals[controller].tx += mode_config.data_frame_size;
    }
}

static inline void _spi_receive(enum SPIController controller, struct spi_mode_config_t mode_config) {
  uint32_t rx_left = (_spi_internals[controller].rx_end - _spi_internals[controller].rx) / mode_config.data_frame_size;
  uint32_t rx_room = _read_mem(_rk3288_spi_block_base + ALIGN(_spi_configs[controller].spi_block_offset + ROCKCHIP_SPI_RXFLR));

  uint32_t max = _min(rx_left, rx_room);
  uint32_t rxw;

  while (max--) {
    rxw = _read_mem(_rk3288_spi_block_base + ALIGN(_spi_configs[controller].spi_block_offset + ROCKCHIP_SPI_RXDR));
    if (mode_config.data_frame_size == DFS_8) {
      *(uint8_t *) (_spi_internals[controller].rx) = (uint8_t) rxw;
    }
    else {
      *(uint16_t *) (_spi_internals[controller].rx) = (uint16_t) rxw;
    }
    
    _spi_internals[controller].rx += mode_config.data_frame_size;
  }
}

void tinkerboard_spi_init(enum SPIController controller, struct spi_mode_config_t mode_config) {

  // Setting IOMUX for tx, rx and clock so that the spi controller is driving these pins
  uint32_t pin = _spi_configs[controller].clk;
  _set_config(_rk3288_gpio_block_base + ALIGN(_gpio_header_pins[pin].grf_bank_offset),
              _gpio_header_pins[pin].grf_pin_offset, 1, _gpio_header_pins[pin].grf_config_size);
  _gpio_header_pins[pin].mode = SPI;

  pin = _spi_configs[controller].txd;
  _set_config(_rk3288_gpio_block_base + ALIGN(_gpio_header_pins[pin].grf_bank_offset),
              _gpio_header_pins[pin].grf_pin_offset, 1, _gpio_header_pins[pin].grf_config_size);
  _gpio_header_pins[pin].mode = SPI;

  pin = _spi_configs[controller].rxd;
  _set_config(_rk3288_gpio_block_base + ALIGN(_gpio_header_pins[pin].grf_bank_offset),
              _gpio_header_pins[pin].grf_pin_offset, 1, _gpio_header_pins[pin].grf_config_size);
  _gpio_header_pins[pin].mode = SPI;
  

  // If valid pin for slave select is provided set it as output to use it later as ss pin
//  if(mode_config.slave_select_0 != 0 && mode_config.slave_select_0 != _spi_configs[controller].txd &&
//      mode_config.slave_select_0 != _spi_configs[controller].rxd && mode_config.slave_select_0 != _spi_configs[controller].clk) {
    _spi_internals[controller].cs0_pin = mode_config.slave_select_0;
    tinkerboard_set_gpio_mode(_spi_internals[controller].cs0_pin, OUTPUT);
    tinkerboard_set_gpio_state(_spi_internals[controller].cs0_pin, HIGH);
	
 // }
  
//   if(mode_config.slave_select_1 != 0 && mode_config.slave_select_1 != _spi_configs[controller].txd &&
//      mode_config.slave_select_1 != _spi_configs[controller].rxd && mode_config.slave_select_1 != _spi_configs[controller].clk) {
    _spi_internals[controller].cs1_pin = mode_config.slave_select_1;
    tinkerboard_set_gpio_mode(_spi_internals[controller].cs1_pin, OUTPUT);
    tinkerboard_set_gpio_state(_spi_internals[controller].cs1_pin, HIGH);
 // }
 
	_spi_internals[controller].cs2_pin = mode_config.slave_select_2;
    tinkerboard_set_gpio_mode(_spi_internals[controller].cs2_pin, OUTPUT);
    tinkerboard_set_gpio_state(_spi_internals[controller].cs2_pin, HIGH);
 

  // Select PLL clock source (codec clock) and set divider value
  _set_config(_rk3288_cru_block_base + ALIGN(_spi_configs[controller].pll_sel_offset), 0, 0x02, 8);

  // Activate clock source
  _set_config(_rk3288_cru_block_base + ALIGN(_spi_configs[controller].clk_src_offset),
              _spi_configs[controller].clk_gate_flag, 0, 1);

  // Activate peripheral clock source
  _set_config(_rk3288_cru_block_base + ALIGN(_spi_configs[controller].pclk_src_offset),
              _spi_configs[controller].pclk_gate_flag, 0, 1);

  // Disable softreset of spi controller
  _set_config(_rk3288_cru_block_base + ALIGN(_spi_configs[controller].softrst_offset),
              _spi_configs[controller].softrst_flag, 0, 1);

  // Assemble configuration register of spi controller
  uint32_t config = 0;
  config |= mode_config.data_frame_size << CR0_DFS_OFFSET;
  config |= mode_config.clk_mode << CR0_SCPH_OFFSET;
  config |= mode_config.byte_order << CR0_FBM_OFFSET;
  config |= 1 << CR0_BHT_OFFSET;
  config |= mode_config.transfer_mode << CR0_XFM_OFFSET;

  _spi_set_ctrlr0(controller, config);
  _spi_set_fifo_size(controller , 31);
  _spi_set_slave_select(controller);
  _spi_set_clk_divider(controller, mode_config.clk_divider);
  _spi_enable_controller(controller, 1);

  _spi_internals[controller].fifo_len = 31;
  _spi_configs[controller].initialized = 1;
}

void tinkerboard_spi_end(enum SPIController controller){
  _spi_enable_controller(controller, 0);
}

void tinkerboard_spi_transfer(enum SPIController controller, uint8_t* tx_buff, uint8_t* rx_buff, uint32_t length, struct spi_mode_config_t mode_config, enum SlaveSelect slave_select ) {
 _spi_enable_controller(controller, 1);


  unsigned long remain = 0;
  _spi_internals[controller].tx = tx_buff;
  _spi_internals[controller].tx_end = tx_buff + length;
  _spi_internals[controller].rx = rx_buff;
  _spi_internals[controller].rx_end = rx_buff + length;

	
	if (slave_select == SS_0) { 
		tinkerboard_set_gpio_state(_spi_internals[controller].cs0_pin, LOW);
	} else if (slave_select == SS_1) {
		tinkerboard_set_gpio_state(_spi_internals[controller].cs1_pin, LOW);
	} else if (slave_select == SS_2) {
		tinkerboard_set_gpio_state(_spi_internals[controller].cs2_pin, LOW);
	}

  do {
    if(_spi_internals[controller].tx) {
      remain = _spi_internals[controller].tx_end - _spi_internals[controller].tx;
      _spi_send(controller, mode_config);
    }

    if(_spi_internals[controller].rx) {
      remain = _spi_internals[controller].rx_end - _spi_internals[controller].rx;
      _spi_receive(controller, mode_config);
    }
  } while (remain);

  // If spi controller is still busy keep waiting and eventually timing out
  if(_spi_internals[controller].tx) {
    _spi_wait_for_idle(controller);
  }

	if (slave_select == SS_0) { 
		tinkerboard_set_gpio_state(_spi_internals[controller].cs0_pin, HIGH);
	} else if (slave_select == SS_1) {
		tinkerboard_set_gpio_state(_spi_internals[controller].cs1_pin, HIGH);
	} else if (slave_select == SS_2) {
		tinkerboard_set_gpio_state(_spi_internals[controller].cs2_pin, HIGH);
	}
	_spi_enable_controller(controller, 0);
}
