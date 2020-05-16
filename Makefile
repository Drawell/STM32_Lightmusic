BUILD_ROOT := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))


SPL_PATH = $(BUILD_ROOT)/stm_spl/
COMMON_PATH = $(BUILD_ROOT)/common
LIB_PATH = $(BUILD_ROOT)/tm_usart
MAIN_DIR = $(BUILD_ROOT)/lightmusic

.PHONY: labs spl common lab1 clean

all: main

main: spl common lib
	make -C $(MAIN_DIR)

flash:
	make -C $(MAIN_DIR) flash

spl:
	make -C $(SPL_PATH)

common:
	make -C $(COMMON_PATH)

lib:
	make -C $(LIB_PATH)

clean:
	make -C $(MAIN_DIR) clean
	make -C $(LIB_PATH) clean

clean_all:
	make -C $(SPL_PATH) clean	
	make -C $(COMMON_PATH) clean
	make -C $(LIB_PATH) clean
	make -C $(LIB_PATH)/led_test clean
