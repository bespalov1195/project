BUILD_ROOT=../..
OBJS = init.o main.o

BINARY_NAME = main

include $(BUILD_ROOT)/common_defs.mk

.PHONY: $(BINARY_NAME)

$(BINARY_NAME): $(BINARY_NAME).bin

clean:
	@rm -f $(OBJS) main.elf main.bin main.elf.map init.d init.o main.d main.o

flash: $(BINARY_NAME)
	$(FLASH_BINARY)
	$(RESET_MCU)


