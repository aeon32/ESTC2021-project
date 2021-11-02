include paths.mk

BLINKY_PATH = $(BUILD_ROOT)/projects/blinky/

all: blinky

blinky: 
	make -C $(BLINKY_PATH)

clean:
	make -C $(BLINKY_PATH) clean







