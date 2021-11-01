include ../../paths.mk
include ../../common_defs.mk


PROJECT_NAME     := blinky
OUTPUT_DIRECTORY := _build
OBJ_DIRECTORY := $(OUTPUT_DIRECTORY)/obj


BINARY_PATH = $(OUTPUT_DIRECTORY)/$(PROJECT_NAME)



CFLAGS += -D__HEAP_SIZE=8192
CFLAGS += -D__STACK_SIZE=8192
ASMFLAGS += -D__HEAP_SIZE=8192
ASMFLAGS += -D__STACK_SIZE=8192

LINKER_SCRIPT  := blinky_gcc_nrf52.ld
#LDFLAGS += -T$(LINKER_SCRIPT)

#Sources and other dependencies
SRCS := main.c \
  $(SDK_ROOT)/components/libraries/log/src/nrf_log_frontend.c \
  $(SDK_ROOT)/components/libraries/log/src/nrf_log_str_formatter.c \
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
  $(SDK_ROOT)/modules/nrfx/soc/nrfx_atomic.c \
  $(SDK_ROOT)/modules/nrfx/mdk/system_nrf52840.c \

OBJS :=



.PHONY: $(PROJECT_NAME)
$(PROJECT_NAME): $(BINARY_PATH)


$(foreach src_file,$(SRCS),\
   $(eval obj_file := $(OBJ_DIRECTORY)/$(notdir $(src_file).o) )\
   $(eval $(COMPILE_RULE) )\
   $(eval OBJS += $(obj_file) )\
) 









$(OUTPUT_DIRECTORY):
	mkdir -p $(OUTPUT_DIRECTORY)
	mkdir -p $(OBJ_DIRECTORY)

$(BINARY_PATH): $(OUTPUT_DIRECTORY) $(OBJS) $(LDLIBS_PATHS)
	$(LD) $(LDFLAGS) -o $@ $(OBJS) $(LIB_FILES)	

clean:
	@rm -rf $(OUTPUT_DIRECTORY)





