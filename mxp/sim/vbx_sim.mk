#include sources.mk
exec=app
#SIM_DIR=./sim/lib
VECTORBLOX_SIM_LICENSE=./sim/license.lic
export VECTORBLOX_SIM_LICENSE
CC=gcc
#CFLAGS= $(shell pkg-config --cflags opencv) -Lsim/lib/vbxsim/ -lvbxsim -Isim/lib/vbxapi/ -Lsim/lib/vbxapi/ -lvbxapi -I$(SIM_DIR) -I$(MXP_FRWK_INC_DIR) $(UNITY_INC_DIR) -DVBX_SIMULATOR $(CMACRO_DEFS) -std=c99 -g
CFLAGS= -Lsim/lib/vbxsim/ -lvbxsim -Isim/lib/vbxapi/ -Lsim/lib/vbxapi/ -lvbxapi -I$(SIM_DIR) -I$(MXP_FRWK_INC_DIR) $(UNITY_INC_DIR) -DVBX_SIMULATOR $(CMACRO_DEFS) -std=c99 -g
LIBS=$(shell pkg-config --libs opencv)

#C_OBJS := $(patsubst %.c,obj/%.o,$(C_SRCS))
OBJ_DIR = obj
C_OBJS := $(addprefix $(OBJ_DIR)/,$(notdir $(patsubst %.c,%.o,$(filter %.c,$(C_SRCS)))))

#libraries
.phony: all
all: obj $(exec)
sim/lib/vbxapi/libvbxapi.a: $(wildcard sim/lib/vbxapi/*.[ch])
	make -C sim/lib/vbxapi

sim/lib/vbxsim/libvbxsim.a:
	make -C sim/lib/vbxsim
vpath %.c $(sort $(dir $(C_SRCS)))

$(C_OBJS): $(OBJ_DIR)/%.o: %.c
	@echo Building $< ......
	@echo $(CFLAGS)
	$(CC) -c $(CFLAGS) $< -o $@

#executable
$(exec) : $(C_OBJS) sim/lib/vbxsim/libvbxsim.a sim/lib/vbxapi/libvbxapi.a
	@echo Linking.......
	$(CC)  -o $@ $(C_OBJS) $(CFLAGS) -lm -lopencv_highgui -lopencv_imgproc -lopencv_objdetect -lopencv_core -lopencv_imgcodecs

obj:
	@mkdir -p $(OBJ_DIR)
print:
	@echo "SIM_DIR=$(SIM_DIR)"
	@echo "C_SRCS = $(C_SRCS)"
	@echo "OBJS = $(C_OBJS)"
	@echo "CFLAGS = $(CFLAGS)"

run: obj $(exec)
	./$(exec)
.phony:clean
clean:
	rm -rf ./$(OBJ_DIR)
	rm -rf $(exec)
#	make -C lib/vbxsim clean
#	make -C lib/vbxapi clean
