include paths.mk

BLINKY_PATH = $(BUILD_ROOT)/projects/blinky/
BASE_ADVERTS_PATH = $(BUILD_ROOT)/projects/base_adverts/
BLINKY_SERVICE_PATH = $(BUILD_ROOT)/projects/blinky_service/

all: blinky base_adverts blinku_service

blinky: 
	make -C $(BLINKY_PATH)

base_adverts: 
	make -C $(BASE_ADVERTS_PATH)	

blinky_service: 
	make -C $(BLINKY_SERVICE_PATH)	

clean:
	make -C $(BLINKY_PATH) clean
	make -C $(BASE_ADVERTS_PATH) clean
	make -C $(BLINKY_SERVICE_PATH) clean






