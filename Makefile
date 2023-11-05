KR ?= $(shell uname -r)
ID = $(shell id -u)
RPI = rpi-4
DRIVER = SBC/$(RPI)/device_driver/driver/radioberry.ko
FIRMWARE = SBC/$(RPI)/device_driver/firmware/radioberry
PIHPSDR = pihpsdr/pigpsdr
WDSP = wdsp/libwdsp.so
FPGATYPE = 
INSTALL = install
DESTDIR = /usr/local/bin
KERNEL_HEADERS=$(DESTDIR)/lib/modules/$(KR)/build
MAKEFLAGS += --no-print-directory

export INSTALL DESTDIR KERNEL_HEADERS

.PHONY: all check-if-root clean clean-driver clean-firmware clean-pihpsdr \
		wdsp driver firmware pihpsdr \
		install install-firmware install-driver install-service stop-radioberry  \

all: driver firmware pihpsdr wdsp

driver: $(DRIVER)

$(DRIVER): $(dir $(DRIVER))
# Sub-make does not work here
	bash -c 'cd SBC/$(RPI)/device_driver/driver && make'

firmware: $(FIRMWARE)

clean-firmware:
	$(MAKE) -C SBC/$(RPI)/device_driver/firmware clean

pihpsdr: $(PIHPSDR)

wdsp: $(WDSP)

clean: clean-pihpsdr clean-wdsp clean-firmware

clean-pihpsdr:
	$(MAKE) -C pihpsdr clean

clean-wdsp:
	$(MAKE) -C wdsp clean

$(FIRMWARE): $(dir $(FIRMWARE))
	$(MAKE) -C SBC/$(RPI)/device_driver/firmware

# TODO build phpsdr with not-yet installed wdsp
$(PIHPSDR): $(WDSP) $(dir $(PIHPSDR))
	+$(MAKE) -C pihpsdr GPIO_INCLUDE= PURESIGNAL_INCLUDE= MIDI_INCLUDE=MIDI

$(WDSP): $(dir $(WDSP))
	+$(MAKE) -C wdsp

install: check-if-root stop-radioberry install-driver install-firmware install-service install-software

%/:
	@mkdir -p $@

install-firmware: firmware check-if-root stop-radioberry
ifeq ($(FPGATYPE),)
	$(error FPGATYPE is required, specify FPGATYPE=CL016 or FPGATYPE=CL025)
endif 
	$(INSTALL) SBC/$(RPI)/releases/dev/$(FPGATYPE)/radioberry.rbf /lib/firmware
	$(info Radioberry gateware Cyclone 10 $(FPGATYPE) installed.)

install-service: check-if-root stop-radioberry
	$(INSTALL) SBC/$(RPI)/device_driver/systemd/radioberry.service /etc/systemd/system/
	$(INSTALL) SBC/$(RPI)/device_driver/systemd/radioberryd /etc/init.d/
	@echo chmod +x /etc/init.d/radioberryd

install-driver: check-if-root $(DRIVER)
	$(PRE_INSTALL)    # Pre-install commands follow.
	$(if $(DESTDIR),,-systemctl stop radioberry)
	$(if $(DESTDIR),,-systemctl disable radioberry)
	$(if $(DESTDIR),,-modprobe -r radioberry)
	$(NORMAL_INSTALL)  # Normal commands follow.
	echo $(MAKE) -C SBC/$(RPI)/device_driver/driver install
	$(POST_INSTALL)    # Post-install commands follow.
	$(if $(DESTDIR),,insmod $(DRIVER))
	$(if $(DESTDIR),,chmod 666 /dev/radioberry)

install-software: check-if-root
	$(MAKE) -C pihpsdr install GPIO_INCLUDE= PURESIGNAL_INCLUDE= MIDI_INCLUDE=MIDI
	$(MAKE) -C wdsp install

check-if-root:
ifeq ($(DESTDIR),)
ifneq ($(ID),0)
	$(warning root privileges are missing)
endif
endif
	
stop-radioberry: check-if-root
	$(if $(DESTDIR),,-systemctl stop radioberry)
	$(if $(DESTDIR),,-systemctl disable radioberry)
	$(if $(DESTDIR),,-modprobe -r radioberry)

.SECONDARY: $(DRIVER) $(FIRMWARE) $(PIHPSDR)
