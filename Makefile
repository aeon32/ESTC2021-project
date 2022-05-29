include paths.mk

BLINKY_PATH = $(BUILD_ROOT)/projects/blinky/
BASE_ADVERTS_PATH = $(BUILD_ROOT)/projects/base_adverts/
BLINKY_SERVICE_PATH = $(BUILD_ROOT)/projects/blinky_service/
REMOTE_BLINKY_PATH = $(BUILD_ROOT)/projects/remote_blinky/

all: blinky base_adverts blinky_service remote_blinky

blinky: 
	make -C $(BLINKY_PATH)

base_adverts: 
	make -C $(BASE_ADVERTS_PATH)	

blinky_service: 
	make -C $(BLINKY_SERVICE_PATH)	

remote_blinky: 
	make -C $(REMOTE_BLINKY_PATH)	
clean:
	make -C $(BLINKY_PATH) clean
	make -C $(BASE_ADVERTS_PATH) clean
	make -C $(BLINKY_SERVICE_PATH) clean
	make -C $(REMOTE_BLINKY_PATH) clean





