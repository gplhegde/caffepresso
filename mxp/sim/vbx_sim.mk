#include sources.mk
MAKEFILE_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
exec=app
CC=gcc
#CFLAGS= $(shell pkg-config --cflags opencv) -Lsim/lib/vbxsim/ -lvbxsim -Isim/lib/vbxapi/ -Lsim/lib/vbxapi/ -lvbxapi -I$(SIM_DIR) -I$(MXP_FRWK_INC_DIR) $(UNITY_INC_DIR) -DVBX_SIMULATOR $(CMACRO_DEFS) -std=c99 -g
SIM_LIB_DIR :=$(MAKEFILE_DIR)lib
INC_DIR := $(wildcard $(SIM_LIB_DIR)/*)
INC_DIR_FLAGS := $(addprefix -I, $(INC_DIR)) -I$(SIM_DIR) -I$(MXP_FRWK_INC_DIR)
LIB_DIR_FLAGS := $(addprefix -L, $(wildcard $(SIM_LIB_DIR)/*))

#CFLAGS= -Lsim/lib/vbxsim/ -lvbxsim -Isim/lib/vbxapi/ -Lsim/lib/vbxapi/ -lvbxapi -I$(SIM_DIR) -I$(MXP_FRWK_INC_DIR) $(UNITY_INC_DIR) -DVBX_SIMULATOR $(CMACRO_DEFS) -std=c99 -g
CFLAGS=  $(INC_DIR_FLAGS) $(LIB_DIR_FLAGS) -lvbxsim -lvbxapi -DVBX_SIMULATOR $(CMACRO_DEFS) -std=c99 -g
LIBS=$(shell pkg-config --libs opencv)

OBJ_DIR = obj
C_OBJS := $(addprefix $(OBJ_DIR)/,$(notdir $(patsubst %.c,%.o,$(filter %.c,$(C_SRCS)))))

#libraries
.phony: all
all: obj $(exec)

vpath %.c $(sort $(dir $(C_SRCS)))

$(C_OBJS): $(OBJ_DIR)/%.o: %.c
	@echo Building $< ......
	@echo $(CFLAGS)
	$(CC) -c $(CFLAGS) $< -o $@

$(exec) : $(C_OBJS)
	@echo Linking.......
	$(CC)  -o $@ $(C_OBJS) $(CFLAGS) -lm -lopencv_highgui -lopencv_imgproc -lopencv_objdetect -lopencv_core -lopencv_imgcodecs

obj:
	@mkdir -p $(OBJ_DIR)
print:
	@echo "C_SRCS = $(C_SRCS)"
	@echo "OBJS = $(C_OBJS)"
	@echo "CFLAGS = $(CFLAGS)"

run: obj $(exec)
	./$(exec)
.phony:clean
clean:
	rm -rf ./$(OBJ_DIR)
	rm -rf $(exec)
