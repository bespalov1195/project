ifneq ($(V),1)
	Q    := @
	NULL := 2>/dev/null
endif

###############################################################################
#                                 Executables                                 #
###############################################################################
PREFIX    ?= arm-none-eabi

CC        := $(PREFIX)-gcc
CXX       := $(PREFIX)-g++
LD        := $(PREFIX)-gcc
AR        := $(PREFIX)-ar
AS        := $(PREFIX)-as
OBJCOPY   := $(PREFIX)-objcopy
OBJDUMP   := $(PREFIX)-objdump
GDB       := $(PREFIX)-gdb
STFLASH    = $(shell which st-flash)

###############################################################################
#                             Include directories                             #
###############################################################################
INCLUDE_DIRS += -I$(BUILD_ROOT)/stm_spl/CMSIS/inc
INCLUDE_DIRS += -I$(BUILD_ROOT)/stm_spl/CMSIS/ST/inc
INCLUDE_DIRS += -I$(BUILD_ROOT)/stm_spl/CMSIS/ST/inc
INCLUDE_DIRS += -I$(BUILD_ROOT)/stm_spl/STM32F4xx/inc
INCLUDE_DIRS += -I$(BUILD_ROOT)/common/include

###############################################################################
#                                  C Defines                                  #
###############################################################################
DEFINES += -DUSE_STDPERIPH_DRIVER -DSTM32F40_41xxx -DHSE_VALUE=8000000

###############################################################################
#                                   C Flags                                   #
###############################################################################
CFLAGS += -O0 -g -MD
CFLAGS += -Wall -Wextra -Wshadow -Wimplicit-function-declaration
CFLAGS += -Wredundant-decls -Wmissing-prototypes -Wstrict-prototypes
CFLAGS += -fno-common -ffunction-sections -fdata-sections

CFLAGS += $(INCLUDE_DIRS) $(DEFINES)

###############################################################################
#                                 Linker flags                                #
###############################################################################
LD_SCRIPT  = $(BUILD_ROOT)/common/ld/stm32f4xx_flash.ld
LIB_DIRS  += $(BUILD_ROOT)/common/ld
LIB_DIRS  += $(BUILD_ROOT)/stm_spl/
LIB_DIRS  += $(BUILD_ROOT)/common/

LDFLAGS += $(addprefix -L, $(LIB_DIRS))

LDFLAGS += --static -nostartfiles
LDFLAGS += -T$(LD_SCRIPT)
LDFLAGS += -Wl,-Map=$@.map
LDFLAGS += -Wl,--gc-sections

ifeq ($(V),1)
	LDFLAGS += -Wl,--print-gc-sections
endif

###############################################################################
#                                Used Libraries                               #
###############################################################################
LDLIBS += -Wl,--start-group -lc -lgcc -lnosys -Wl,--end-group
LDLIBS += -lstmcommon
LDLIBS += -lstm_spl

STARTUP_S      = $(BUILD_ROOT)/common/as/startup_stm32f40_41xxx.S
STARTUP        = ${STARTUP_S:.S=.o}

###############################################################################
#                      Processor-specific compiler flags                      #
###############################################################################
FP_FLAGS   = -mfloat-abi=hard -mfpu=fpv4-sp-d16
ARCH_FLAGS = -mthumb -mcpu=cortex-m4 $(FP_FLAGS)

###############################################################################
#                          Instructions to link files                         #
###############################################################################
LINK_ELF     = $(Q)$(LD) $(LDFLAGS) $(ARCH_FLAGS) $(OBJS) $(STARTUP) $(LDLIBS) -o $@
MAKE_IMAGE   = $(Q)$(OBJCOPY) -Obinary $? $@
MAKE_LIBRARY = $(Q)$(AR) r $@ $^

###############################################################################
#                     Flashing and debugging instructions                     #
###############################################################################
FLASH_BINARY = $(Q)$(STFLASH) write $?.bin 0x8000000

###############################################################################
#                                 Build rules                                 #
###############################################################################
.SUFFIXES: .elf
.SECONDEXPANSION:
.SECONDARY:

.PHONY: clean

# Assembly build rules
%.o: %.S
	@echo " AS" $<
	$(Q)$(AS) $< -o $@

# C files build rules
%.o: %.c
	@echo " CC" $<
	$(Q)$(CC) $(CFLAGS) $(CPPFLAGS) $(ARCH_FLAGS) -o $@ -c $<

%.elf: $(OBJS) $(STARTUP)
	@echo " LD" $^
	$(LINK_ELF)

%.bin: %.elf
	@echo " OBJCOPY" $^
	$(MAKE_IMAGE)
