BUILD_ROOT = ../../..

OBJS = led.o
DEPS = led.d

include $(BUILD_ROOT)/common_defs.mk

LIB = libled.a

all: $(LIB)
	@echo "Done"

$(LIB): $(OBJS)
	@echo "Linking library $@"
	$(MAKE_LIBRARY)

clean:
	@echo "Cleaning library directory"
	@rm -f $(OBJS) $(DEPS) $(LIB)
