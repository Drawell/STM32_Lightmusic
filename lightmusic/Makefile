BUILD_ROOT=../
#OBJS = main.o

BINARY_NAME = main

include $(BUILD_ROOT)/common_defs.mk

SRC_DIR = ./src

SRCS = $(wildcard  $(addsuffix /*.c, $(SRC_DIR)))
OBJS = $(patsubst %.c, %.o, $(SRCS))
DEPS = $(patsubst %.c, %.d, $(SRCS))

.PHONY: $(BINARY_NAME)

$(BINARY_NAME): $(OBJS) $(BINARY_NAME).bin

clean:
	@rm -f $(OBJS) $(DEPS) main.elf main.bin main.elf.map

flash: $(BINARY_NAME)
	$(FLASH_BINARY)