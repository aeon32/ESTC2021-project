include paths.mk

BLINKY_PATH = $(BUILD_ROOT)/projects/blinky/
BLINKY_NRF_LOG_PATH = $(BUILD_ROOT)/projects/blinky_nrf_log/

all: blinky

blinky: 
	make -C $(BLINKY_PATH)
	make -C $(BLINKY_NRF_LOG_PATH)
clean:
	make -C $(BLINKY_PATH) clean
	make -C $(BLINKY_NRF_LOG_PATH) clean






