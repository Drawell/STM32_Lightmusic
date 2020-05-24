BUILD_ROOT := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))


SPL_PATH = $(BUILD_ROOT)/stm_spl/
COMMON_PATH = $(BUILD_ROOT)/common/
LIB_PATH = $(BUILD_ROOT)/signal_processor/
MAIN_DIR = $(BUILD_ROOT)/lightmusic/

.PHONY: labs spl common lib clean

all: main

main: spl lib common
	make -C $(MAIN_DIR)

flash: spl lib common
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
	make -C $(MAIN_DIR) clean
	make -C $(LIB_PATH) clean
