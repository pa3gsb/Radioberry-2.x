// ---- GPIO specific defines
#define GPIO_REGISTER_BASE 0x200000
#define GPIO_SET_OFFSET 0x1C
#define GPIO_CLR_OFFSET 0x28
#define GPIO_LEV0_OFFSET 0x34
#define PHYSICAL_GPIO_BUS (0x7E000000 + GPIO_REGISTER_BASE)

#define PAGE_SIZE 4096
#define BCM2708_PI1_PERI_BASE  0x20000000
#define BCM2709_PI2_PERI_BASE  0x3F000000
#define BCM2711_PI4_PERI_BASE  0xFE000000

#define PERI_BASE BCM2711_PI4_PERI_BASE

volatile uint32_t *rpi_io=NULL; 
volatile uint32_t *rpi_set_io_high=NULL; 
volatile uint32_t *rpi_set_io_low=NULL;
volatile uint32_t *rpi_read_io=NULL; 



// Return a pointer to a periphery subsystem register.
static void *mmap_bcm_register(off_t register_offset) {
  const off_t base = PERI_BASE;

  int mem_fd;
  if ((mem_fd = open("/dev/mem", O_RDWR|O_SYNC) ) < 0) {
    perror("can't open /dev/mem: ");
    fprintf(stderr, "You need to run this as root!\n");
    return NULL;
  }

  uint32_t *result =
    (uint32_t*) mmap(NULL,                  // Any adddress in our space will do
                     PAGE_SIZE,
                     PROT_READ|PROT_WRITE,  // Enable r/w on GPIO registers.
                     MAP_SHARED,
                     mem_fd,                // File to map
                     base + register_offset // Offset to bcm register
                     );
  close(mem_fd);

  if (result == MAP_FAILED) {
    fprintf(stderr, "mmap error %p\n", result);
    return NULL;
  }
  return result;
}

void initialize_gpio_for_output(volatile uint32_t *gpio_registerset, int bit) {
  *(gpio_registerset+(bit/10)) &= ~(7<<((bit%10)*3));  // prepare: set as input
  *(gpio_registerset+(bit/10)) |=  (1<<((bit%10)*3));  // set as output.
}

void initialize_gpio_for_input(volatile uint32_t *gpio_registerset, int bit) {
  *(gpio_registerset+(bit/10)) &= ~(7<<((bit%10)*3));  // prepare: set as input
}


void init_RPI_GPIO() {
	// Prepare GPIO
	volatile uint32_t *gpio_port = mmap_bcm_register(GPIO_REGISTER_BASE);
	rpi_io = gpio_port;
	volatile uint32_t *set_reg = gpio_port + (GPIO_SET_OFFSET / sizeof(uint32_t));
	rpi_set_io_high = set_reg;
	volatile uint32_t *clr_reg = gpio_port + (GPIO_CLR_OFFSET / sizeof(uint32_t));
	rpi_set_io_low = clr_reg;
	volatile uint32_t *read_reg = gpio_port + (GPIO_LEV0_OFFSET / sizeof(uint32_t));
	rpi_read_io = read_reg;
	
	//RX IO Init part 
	initialize_gpio_for_output(gpio_port, RPI_RX_CLK);
	*clr_reg = (1<<RPI_RX_CLK); 	// init pi-rx_clk
	
	initialize_gpio_for_input(gpio_port, 21);	// rx iq data
	initialize_gpio_for_input(gpio_port, 20);	// rx iq data
	initialize_gpio_for_input(gpio_port, 19);	// rx iq data
	initialize_gpio_for_input(gpio_port, 16);	// rx iq data
	initialize_gpio_for_input(gpio_port, 25);	// available samples.
	
	initialize_gpio_for_input(gpio_port, 23);	// last
	
	//TX IO Init part 
	initialize_gpio_for_output(gpio_port, RPI_TX_CLK);
	*clr_reg = (1<<RPI_TX_CLK); 	// init pi-tx_clk
	initialize_gpio_for_output(gpio_port, 5);	// tx iq data
	initialize_gpio_for_output(gpio_port, 12);	// tx iq data
	initialize_gpio_for_output(gpio_port, 17);	// tx iq data
	initialize_gpio_for_output(gpio_port, 18);	// tx iq data
}


