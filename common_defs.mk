###############################################################################
#                                 Executables                                 #
###############################################################################
GNU_INSTALL_ROOT ?= /usr/local/gcc-arm-none-eabi-9-2020-q2-update/bin/
GNU_VERSION ?= 9.3.1
GNU_PREFIX ?= arm-none-eabi
PROJ_DIR = $(dir $(abspath $(firstword $(MAKEFILE_LIST))))


PREFIX    ?= $(GNU_INSTALL_ROOT)/$(GNU_PREFIX)-
CC        := $(PREFIX)gcc
CXX       := $(PREFIX)g++
LD        := $(PREFIX)gcc
AR        := $(PREFIX)ar
OBJCOPY   := $(PREFIX)objcopy
###############################################################################
#                             Include directories                             #
###############################################################################
INCLUDE_DIRS += $(BUILD_ROOT)\
  $(PROJ_DIR)\
  $(SDK_ROOT)/components \
  $(SDK_ROOT)/components/ble/ble_advertising \
  $(SDK_ROOT)/components/ble/ble_dtm \
  $(SDK_ROOT)/components/ble/ble_racp \
  $(SDK_ROOT)/components/ble/ble_services/ble_ancs_c \
  $(SDK_ROOT)/components/ble/ble_services/ble_ans_c \
  $(SDK_ROOT)/components/ble/ble_services/ble_bas \
  $(SDK_ROOT)/components/ble/ble_services/ble_bas_c \
  $(SDK_ROOT)/components/ble/ble_services/ble_cscs \
  $(SDK_ROOT)/components/ble/ble_services/ble_cts_c \
  $(SDK_ROOT)/components/ble/ble_services/ble_dfu \
  $(SDK_ROOT)/components/ble/ble_services/ble_dis \
  $(SDK_ROOT)/components/ble/ble_services/ble_gls \
  $(SDK_ROOT)/components/ble/ble_services/ble_hids \
  $(SDK_ROOT)/components/ble/ble_services/ble_hrs \
  $(SDK_ROOT)/components/ble/ble_services/ble_hrs_c \
  $(SDK_ROOT)/components/ble/ble_services/ble_hts \
  $(SDK_ROOT)/components/ble/ble_services/ble_ias \
  $(SDK_ROOT)/components/ble/ble_services/ble_ias_c \
  $(SDK_ROOT)/components/ble/ble_services/ble_lbs \
  $(SDK_ROOT)/components/ble/ble_services/ble_lbs_c \
  $(SDK_ROOT)/components/ble/ble_services/ble_lls \
  $(SDK_ROOT)/components/ble/ble_services/ble_nus \
  $(SDK_ROOT)/components/ble/ble_services/ble_nus_c \
  $(SDK_ROOT)/components/ble/ble_services/ble_rscs \
  $(SDK_ROOT)/components/ble/ble_services/ble_rscs_c \
  $(SDK_ROOT)/components/ble/ble_services/ble_tps \
  $(SDK_ROOT)/components/ble/common \
  $(SDK_ROOT)/components/ble/nrf_ble_gatt \
  $(SDK_ROOT)/components/ble/nrf_ble_qwr \
  $(SDK_ROOT)/components/ble/peer_manager \
  $(SDK_ROOT)/components/boards \
  $(SDK_ROOT)/components/libraries/atomic \
  $(SDK_ROOT)/components/libraries/atomic_fifo \
  $(SDK_ROOT)/components/libraries/atomic_flags \
  $(SDK_ROOT)/components/libraries/balloc \
  $(SDK_ROOT)/components/libraries/bootloader/ble_dfu \
  $(SDK_ROOT)/components/libraries/bsp \
  $(SDK_ROOT)/components/libraries/button \
  $(SDK_ROOT)/components/libraries/cli \
  $(SDK_ROOT)/components/libraries/crc16 \
  $(SDK_ROOT)/components/libraries/crc32 \
  $(SDK_ROOT)/components/libraries/crypto \
  $(SDK_ROOT)/components/libraries/csense \
  $(SDK_ROOT)/components/libraries/csense_drv \
  $(SDK_ROOT)/components/libraries/delay \
  $(SDK_ROOT)/components/libraries/ecc \
  $(SDK_ROOT)/components/libraries/experimental_section_vars \
  $(SDK_ROOT)/components/libraries/experimental_task_manager \
  $(SDK_ROOT)/components/libraries/fds \
  $(SDK_ROOT)/components/libraries/fstorage \
  $(SDK_ROOT)/components/libraries/gfx \
  $(SDK_ROOT)/components/libraries/gpiote \
  $(SDK_ROOT)/components/libraries/hardfault \
  $(SDK_ROOT)/components/libraries/hci \
  $(SDK_ROOT)/components/libraries/led_softblink \
  $(SDK_ROOT)/components/libraries/log \
  $(SDK_ROOT)/components/libraries/log/src \
  $(SDK_ROOT)/components/libraries/log/src\
  $(SDK_ROOT)/components/libraries/low_power_pwm \
  $(SDK_ROOT)/components/libraries/mem_manager \
  $(SDK_ROOT)/components/libraries/memobj \
  $(SDK_ROOT)/components/libraries/mpu \
  $(SDK_ROOT)/components/libraries/mutex \
  $(SDK_ROOT)/components/libraries/pwm \
  $(SDK_ROOT)/components/libraries/pwr_mgmt \
  $(SDK_ROOT)/components/libraries/queue \
  $(SDK_ROOT)/components/libraries/ringbuf \
  $(SDK_ROOT)/components/libraries/scheduler \
  $(SDK_ROOT)/components/libraries/sdcard \
  $(SDK_ROOT)/components/libraries/sensorsim \
  $(SDK_ROOT)/components/libraries/slip \
  $(SDK_ROOT)/components/libraries/sortlist \
  $(SDK_ROOT)/components/libraries/spi_mngr \
  $(SDK_ROOT)/components/libraries/stack_guard \
  $(SDK_ROOT)/components/libraries/strerror \
  $(SDK_ROOT)/components/libraries/svc \
  $(SDK_ROOT)/components/libraries/timer \
  $(SDK_ROOT)/components/libraries/twi_mngr \
  $(SDK_ROOT)/components/libraries/twi_sensor \
  $(SDK_ROOT)/components/libraries/usbd \
  $(SDK_ROOT)/components/libraries/usbd/class \
  $(SDK_ROOT)/components/libraries/usbd/class/audio \
  $(SDK_ROOT)/components/libraries/usbd/class/cdc \
  $(SDK_ROOT)/components/libraries/usbd/class/cdc/acm \
  $(SDK_ROOT)/components/libraries/usbd/class/hid \
  $(SDK_ROOT)/components/libraries/usbd/class/hid/generic \
  $(SDK_ROOT)/components/libraries/usbd/class/hid/kbd \
  $(SDK_ROOT)/components/libraries/usbd/class/hid/mouse \
  $(SDK_ROOT)/components/libraries/usbd/class/msc \
  $(SDK_ROOT)/components/libraries/util \
  $(SDK_ROOT)/components/nfc/ndef/connection_handover/ac_rec \
  $(SDK_ROOT)/components/nfc/ndef/connection_handover/ble_oob_advdata \
  $(SDK_ROOT)/components/nfc/ndef/connection_handover/ble_pair_lib \
  $(SDK_ROOT)/components/nfc/ndef/connection_handover/ble_pair_msg \
  $(SDK_ROOT)/components/nfc/ndef/connection_handover/common \
  $(SDK_ROOT)/components/nfc/ndef/connection_handover/ep_oob_rec \
  $(SDK_ROOT)/components/nfc/ndef/connection_handover/hs_rec \
  $(SDK_ROOT)/components/nfc/ndef/connection_handover/le_oob_rec \
  $(SDK_ROOT)/components/nfc/ndef/conn_hand_parser \
  $(SDK_ROOT)/components/nfc/ndef/conn_hand_parser/ac_rec_parser \
  $(SDK_ROOT)/components/nfc/ndef/conn_hand_parser/ble_oob_advdata_parser \
  $(SDK_ROOT)/components/nfc/ndef/conn_hand_parser/le_oob_rec_parser \
  $(SDK_ROOT)/components/nfc/ndef/generic/message \
  $(SDK_ROOT)/components/nfc/ndef/generic/record \
  $(SDK_ROOT)/components/nfc/ndef/launchapp \
  $(SDK_ROOT)/components/nfc/ndef/parser/message \
  $(SDK_ROOT)/components/nfc/ndef/parser/record \
  $(SDK_ROOT)/components/nfc/ndef/text \
  $(SDK_ROOT)/components/nfc/ndef/uri \
  $(SDK_ROOT)/components/nfc/platform \
  $(SDK_ROOT)/components/nfc/t2t_lib \
  $(SDK_ROOT)/components/nfc/t2t_parser \
  $(SDK_ROOT)/components/nfc/t4t_lib \
  $(SDK_ROOT)/components/nfc/t4t_parser/apdu \
  $(SDK_ROOT)/components/nfc/t4t_parser/cc_file \
  $(SDK_ROOT)/components/nfc/t4t_parser/hl_detection_procedure \
  $(SDK_ROOT)/components/nfc/t4t_parser/tlv \
  $(SDK_ROOT)/components/softdevice/common \
  $(SDK_ROOT)/components/softdevice/mbr/headers \
  $(SDK_ROOT)/components/softdevice/s113/headers \
  $(SDK_ROOT)/components/softdevice/s113/headers/nrf52 \
  $(SDK_ROOT)/components/toolchain/cmsis/include \
  $(SDK_ROOT)/external/fprintf \
  $(SDK_ROOT)/external/segger_rtt \
  $(SDK_ROOT)/external/utf_converter \
  $(SDK_ROOT)/integration/nrfx \
  $(SDK_ROOT)/integration/nrfx/legacy \
  $(SDK_ROOT)/modules/nrfx \
  $(SDK_ROOT)/modules/nrfx/drivers/include \
  $(SDK_ROOT)/modules/nrfx/hal \
  $(SDK_ROOT)/modules/nrfx/mdk

INCS := $(addprefix -I, $(INCLUDE_DIRS) )

###############################################################################
#                             Flags                                           #
###############################################################################
#Optimization flags
OPT = -O3 -g3
# Uncomment the line below to enable link time optimization
#OPT += -flto

# C flags common to all targets
CFLAGS += $(OPT)
CFLAGS += -std=c99  -MP -MD 
CFLAGS += -DBOARD_PCA10059
CFLAGS += -DCONFIG_GPIO_AS_PINRESET
CFLAGS += -DFLOAT_ABI_HARD
CFLAGS += -DMBR_PRESENT
CFLAGS += -DNRF52840_XXAA
CFLAGS += -mcpu=cortex-m4
CFLAGS += -mthumb -mabi=aapcs
CFLAGS += -Wall -Werror
CFLAGS += -mfloat-abi=hard -mfpu=fpv4-sp-d16
# keep every function in a separate section, this allows linker to discard unused ones
CFLAGS += -ffunction-sections -fdata-sections -fno-strict-aliasing
CFLAGS += -fno-builtin -fshort-enums

# C++ flags common to all targets
CXXFLAGS += $(OPT)

# Assembler flags common to all targets
ASMFLAGS += -MP -MD
ASMFLAGS += -x assembler-with-cpp
ASMFLAGS += -g3
ASMFLAGS += -mcpu=cortex-m4
ASMFLAGS += -mthumb -mabi=aapcs
ASMFLAGS += -mfloat-abi=hard -mfpu=fpv4-sp-d16
ASMFLAGS += -DBOARD_PCA10059
ASMFLAGS += -DBSP_DEFINES_ONLY
ASMFLAGS += -DCONFIG_GPIO_AS_PINRESET
ASMFLAGS += -DFLOAT_ABI_HARD
ASMFLAGS += -DMBR_PRESENT
ASMFLAGS += -DNRF52840_XXAA

CFLAGS += $(INCS) 


###############################################################################
#                                 Linker flags                                #
###############################################################################
LIB_DIRS  += $(BIN_DIR)
LDFLAGS += $(OPT)
LDFLAGS += $(addprefix -L, $(LIB_DIRS))
LDFLAGS += -mcpu=cortex-m4
LDFLAGS += -mfloat-abi=hard -mfpu=fpv4-sp-d16
# let linker dump unused sections
LDFLAGS += -Wl,--gc-sections
# use newlib in nano version
LDFLAGS += --specs=nano.specs
# Add standard libraries at the very end of the linker input, after all objects
# that may need symbols provided by these libraries.
LIB_FILES += -lc -lnosys -lm




define COMPILE_RULE
$(obj_file) : Makefile $(src_file)
	@mkdir -p $(OBJ_DIRECTORY)
	$(CC) $(CFLAGS) -MMD -c $(src_file) -o $(obj_file)
endef

define ASM_RULE
$(obj_file) : $(src_file)
	$(CC) $(ASMFLAGS)  -o $(obj_file) -c $(src_file)
endef





