include paths.mk

BLINKY_PATH = $(BUILD_ROOT)/projects/blinky/
BASE_ADVERTS_PATH = $(BUILD_ROOT)/projects/base_adverts/


all: blinky base_adverts

blinky: 
	make -C $(BLINKY_PATH)

base_adverts: 
	make -C $(BASE_ADVERTS_PATH)	

clean:
	make -C $(BLINKY_PATH) clean
	make -C $(BASE_ADVERTS_PATH) clean







