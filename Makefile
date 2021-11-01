include paths.mk

BLINKY_PATH = $(BUILD_ROOT)/projects/blinky/

all: blinky

bin:
	mkdir -p $(BIN_DIR)


blinky: bin  
	make -C $(BLINKY_PATH)

clean:
	make -C $(BLINKY_PATH) clean







