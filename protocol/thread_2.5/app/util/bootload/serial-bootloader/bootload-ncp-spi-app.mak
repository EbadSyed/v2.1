# This Makefile defines how to build 'bootload-ncp-spi' for a unix host.
#

# Variables


GLOBAL_BASE_DIR := ../../platform/base

.SUFFIXES:

ifdef E_CC
  CC = $(E_CC)
endif
LD = $(CC)
STD ?= c99

SHELL = /bin/sh

DEFINES = \
  -DUNIX \
  -DUNIX_HOST \
  -DPHY_NULL \
  -DBOARD_HEADER=\"$(GLOBAL_BASE_DIR)/hal/micro/unix/host/board/host.h\" \
  -DCONFIGURATION_HEADER=\"app/util/bootload/serial-bootloader/bootload-ncp-spi-configuration.h\" \
  -DPLATFORM_HEADER=\"$(GLOBAL_BASE_DIR)/hal/micro/unix/compiler/gcc.h\" \
  -DBOARD_HOST \
  -DEMBER_STACK_IP \
  -DEZSP_HOST \
  -DEZSP_SPI \
  -DNCP_SPI_DEVICE=\"/dev/spidev0.0\" \
  -DNCP_SPI_MODE=0 \
  -DNCP_SPI_SPEED_HZ=1048576 \
  -DNCP_CHIP_SELECT_GPIO=\"8\" \
  -DNCP_HOST_INT_GPIO=\"22\" \
  -DNCP_RESET_GPIO=\"23\" \
  -DNCP_WAKE_GPIO=\"24\" \


INCLUDES = \
  -I. \
  -I$(GLOBAL_BASE_DIR) \
  -I$(GLOBAL_BASE_DIR)/hal \
  -I./app/util \
  -I./stack \


FILES = \
  app/ezsp-host/spi/spi-host-ui.c \
  app/ezsp-host/spi/spi-host.c \
  app/ezsp-host/ezsp-host-ui.c \
  app/util/bootload/serial-bootloader/bootload-ncp-app.c \
  app/util/bootload/serial-bootloader/bootload-ncp-spi.c \
  app/util/bootload/serial-bootloader/bootload-ncp.c \
  app/util/bootload/serial-bootloader/bootload-xmodem.c \
  app/util/serial/simple-linux-serial.c \
  stack/framework/byte-utilities.c \
  $(GLOBAL_BASE_DIR)/hal/micro/unix/host/spi-protocol-linux.c \
  $(GLOBAL_BASE_DIR)/hal/micro/generic/crc.c \
  $(GLOBAL_BASE_DIR)/hal/micro/generic/system-timer.c \


LIBRARIES = \

# Negate -Werror for some specific warnings.  Clang and GCC warn about
# different things, and have different switches, so they each get some
# additional negations.
WNO_ERRORS = \
  -Wno-error=pointer-to-int-cast
ifeq "$(shell $(CC) 2>&1 | grep -iqs clang ; echo $$?)" "0"
  WNO_ERRORS += \
    -Wno-error=tautological-constant-out-of-range-compare
endif

CFLAGS = -ggdb -O0 -std=$(STD) -Wall -Werror $(WNO_ERRORS)
CPPFLAGS = $(DEFINES) $(INCLUDES) -MF $(BUILD_PATH)/$(@F:.o=.d) -MMD -MP

ifeq ($(shell uname -s | sed -e 's/^\(CYGWIN\).*/\1/'),Darwin)
  LDFLAGS =
else
  LDFLAGS = -Wl,-export-dynamic
endif
LDLIBS = -pthread

BUILD_PATH = $(CURDIR)/build/bootload-ncp-spi-app

DEPENDENCIES = $(addprefix $(BUILD_PATH)/, $(notdir $(FILES:.c=.d)))
EXECUTABLE = $(BUILD_PATH)/bootload-ncp-spi-app
OBJECTS = $(addprefix $(BUILD_PATH)/, $(notdir $(FILES:.c=.o)))

VPATH = $(dir $(FILES))

.PHONY: all
all: $(EXECUTABLE)

.PHONY: clean
clean:
	@-rm -f $(DEPENDENCIES) $(EXECUTABLE) $(OBJECTS)

ifneq ($(MAKECMDGOALS),clean)
  -include $(DEPENDENCIES)
endif

$(EXECUTABLE): $(OBJECTS) $(LIBRARIES) | $(BUILD_PATH)
ifeq ($(shell uname -s | sed -e 's/^\(CYGWIN\).*/\1/'),Darwin)
	$(LD) $(LDFLAGS) $^ $(LDLIBS) -o $@
else
	$(LD) $(LDFLAGS) -Xlinker --start-group $^ $(LDLIBS) -Xlinker --end-group -o $@
endif

$(BUILD_PATH)/%.o: %.c | $(BUILD_PATH)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

$(BUILD_PATH):
	@mkdir $(BUILD_PATH)
