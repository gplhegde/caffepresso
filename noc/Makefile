# Make sure this is pointing to the right directory
EPIPHANY_HOME := /opt/adapteva/esdk

ESDK := $(EPIPHANY_HOME)
ELIBS := $(ESDK)/tools/host/lib
EINCS := $(ESDK)/tools/host/include
ELDF := $(ESDK)/bsps/current/fast.ldf

# global parameters
exec := main
OBJ_DIR := obj-files
CC := gcc
MKAEFILE_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
SRC_DIR := ./src
TEST_SRC_DIR := ./test
INC_DIR := $(shell pwd)/include
C_SRCS += main.c
C_SRCS += $(wildcard $(TEST_SRC_DIR)/*.c)
C_SRCS += $(wildcard $(SRC_DIR)/*.c)

INC_DIR_FLAGS := $(addprefix -I, $(INC_DIR))
LIB_DIR_FLAGS := -lpapi
CFLAGS =  $(INC_DIR_FLAGS) $(LIB_DIR_FLAGS) -std=c99 -g
C_OBJS = $(addprefix $(OBJ_DIR)/, $(notdir $(patsubst %.c, %.o, $(filter %.c, $(C_SRCS)))))
vpath %.c $(dir $(C_SRCS))

all: obj $(exec)

$(exec):$(C_OBJS)
	@mkdir -p bin
	@echo "Linking..."
	$(CC) -o ./bin/$@ $(C_OBJS) $(CFLAGS) -lm

$(C_OBJS):$(OBJ_DIR)/%.o:%.c
	@echo "Building $< ..."
	$(CC) -c $(CFLAGS) $< -o $@

obj:
	mkdir -p $(OBJ_DIR)

run: obj $(exec)
	./bin/$(exec)

epiphany:
	@mkdir -p bin
	@e-gcc -finline-functions -ffast-math -T $(ELDF) -I./include/ -O3 e_main.c -o bin/@.elf -le-lib # Build DEVICE side program
	@e-objcopy --srec-forceS3 --output-target srec bin/@.elf bin/@.srec # Convert ebinary to SREC filesdads

.phony: print clean
print:
	@echo "C_SRCS = $(C_SRCS)"
	@echo "C_OBJS = $(C_OBJS)"
	@echo "INC_DIR_FLAGS = $(INC_DIR_FLAGS)"
	@echo "LIB_DIR_FLAGS = $(LIB_DIR_FLAGS)"
	@echo "CFLAGS = $(CFLAGS)"

clean:
	rm -rf $(exec) $(OBJ_DIR) bin

