include ../../paths.mk
include ../../common_defs.mk


PROJECT_NAME     := blinky
OUTPUT_DIRECTORY := _build
OBJ_DIRECTORY := $(OUTPUT_DIRECTORY)/obj


BINARY_PATH = $(OUTPUT_DIRECTORY)/$(PROJECT_NAME)
HEX_PATH = $(OUTPUT_DIRECTORY)/$(PROJECT_NAME).hex


CFLAGS += -DUSE_APP_CONFIG
CFLAGS += -DAPP_TIMER_V2 
CFLAGS += -DAPP_TIMER_V2_RTC1_ENABLED 
CFLAGS += -D__HEAP_SIZE=8192
CFLAGS += -D__STACK_SIZE=8192
ASMFLAGS += -D__HEAP_SIZE=8192
ASMFLAGS += -D__STACK_SIZE=8192
ASMFLAGS += -DAPP_TIMER_V2
ASMFLAGS += -DAPP_TIMER_V2_RTC1_ENABLED

LINKER_SCRIPT  := blinky_gcc_nrf52.ld
LDFLAGS +=  -L$(SDK_ROOT)/modules/nrfx/mdk  -T$(LINKER_SCRIPT)

#Sources and other dependencies
C_SRCS := main.c \
  estc_pwm.c \
  estc_blinky_machine.c\
  estc_monotonic_time.c\
  estc_button.c\
  $(SDK_ROOT)/components/libraries/log/src/nrf_log_frontend.c \
  $(SDK_ROOT)/components/libraries/log/src/nrf_log_str_formatter.c \
  $(SDK_ROOT)/components/libraries/log/src/nrf_log_backend_usb.c \
  $(SDK_ROOT)/components/libraries/log/src/nrf_log_default_backends.c \
  $(SDK_ROOT)/components/libraries/log/src/nrf_log_backend_serial.c \
  $(SDK_ROOT)/components/libraries/usbd/class/cdc/acm/app_usbd_cdc_acm.c \
  $(SDK_ROOT)/components/libraries/usbd/app_usbd.c \
  $(SDK_ROOT)/components/libraries/usbd/app_usbd_core.c \
  $(SDK_ROOT)/components/boards/boards.c \
  $(SDK_ROOT)/components/libraries/util/app_error.c \
  $(SDK_ROOT)/components/libraries/util/app_error_handler_gcc.c \
  $(SDK_ROOT)/components/libraries/util/app_error_weak.c \
  $(SDK_ROOT)/components/libraries/util/app_util_platform.c \
  $(SDK_ROOT)/components/libraries/util/nrf_assert.c \
  $(SDK_ROOT)/components/libraries/atomic/nrf_atomic.c \
  $(SDK_ROOT)/components/libraries/balloc/nrf_balloc.c \
  $(SDK_ROOT)/external/fprintf/nrf_fprintf.c \
  $(SDK_ROOT)/external/fprintf/nrf_fprintf_format.c \
  $(SDK_ROOT)/components/libraries/memobj/nrf_memobj.c \
  $(SDK_ROOT)/components/libraries/ringbuf/nrf_ringbuf.c \
  $(SDK_ROOT)/components/libraries/strerror/nrf_strerror.c \
  $(SDK_ROOT)/components/libraries/button/app_button.c \
  $(SDK_ROOT)/modules/nrfx/soc/nrfx_atomic.c \
  $(SDK_ROOT)/modules/nrfx/mdk/system_nrf52840.c \
  $(SDK_ROOT)/modules/nrfx/drivers/src/nrfx_gpiote.c \
  $(SDK_ROOT)/modules/nrfx/drivers/src/nrfx_usbd.c \
  $(SDK_ROOT)/modules/nrfx/drivers/src/nrfx_systick.c \
  $(SDK_ROOT)/components/libraries/timer/app_timer.c \
  $(SDK_ROOT)/integration/nrfx/legacy/nrf_drv_power.c \
  $(SDK_ROOT)/integration/nrfx/legacy/nrf_drv_clock.c \
  $(SDK_ROOT)/components/libraries/usbd/app_usbd_serial_num.c \
  $(SDK_ROOT)/components/libraries/atomic_fifo/nrf_atfifo.c \
  $(SDK_ROOT)/modules/nrfx/drivers/src/nrfx_power.c \
  $(SDK_ROOT)/modules/nrfx/drivers/src/nrfx_clock.c \
  $(SDK_ROOT)/components/libraries/usbd/app_usbd_string_desc.c \
  $(SDK_ROOT)/components/libraries/bsp/bsp.c \
  $(SDK_ROOT)/components/libraries/sortlist/nrf_sortlist.c 

ASM_SRCS:=$(SDK_ROOT)/modules/nrfx/mdk/gcc_startup_nrf52840.S
OBJS :=



.PHONY: $(PROJECT_NAME)
$(PROJECT_NAME): $(BINARY_PATH) $(HEX_PATH)



$(foreach src_file,$(C_SRCS),\
   $(eval obj_file := $(OBJ_DIRECTORY)/$(notdir $(src_file).o) )\
   $(eval $(COMPILE_RULE) )\
   $(eval OBJS += $(obj_file) )\
) 

$(foreach src_file,$(ASM_SRCS),\
   $(eval obj_file := $(OBJ_DIRECTORY)/$(notdir $(src_file).o) )\
   $(eval $(ASM_RULE) )\
   $(eval OBJS += $(obj_file) )\
) 

$(OUTPUT_DIRECTORY):
	mkdir -p $(OUTPUT_DIRECTORY)
	mkdir -p $(OBJ_DIRECTORY)

$(BINARY_PATH): $(OUTPUT_DIRECTORY) $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $(OBJS)

$(HEX_PATH): $(BINARY_PATH)
	$(OBJCOPY) -O ihex $< $@



.PHONY: flash
# Flash the program
DFU_PORT ?= /dev/ttyACM0

flash: $(HEX_PATH)
	@echo Flashing: $(HEX_PATH)
	nrfutil pkg generate $(OUTPUT_DIRECTORY)/app.zip \
      --hw-version 52 \
      --sd-req 0 \
      --application $(HEX_PATH) \
      --application-version 1 \
      --softdevice $(SDK_ROOT)/components/softdevice/s113/hex/s113_nrf52_7.2.0_softdevice.hex \
      --sd-id 0x0102 \
        >/dev/null
	nrfutil dfu usb_serial \
      --package $(OUTPUT_DIRECTORY)/app.zip \
      --port $(DFU_PORT)


clean:
	@rm -rf $(OUTPUT_DIRECTORY)





