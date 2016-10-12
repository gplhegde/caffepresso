# Get directory where this file resides
MAKEFILE_DIR := $(dir $(lastword $(MAKEFILE_LIST)))

include $(realpath $(MAKEFILE_DIR)/xil_vars.mk)

ALT_INCLUDE_DIRS = $(wildcard $(MAKEFILE_DIR)../vbxinc/*)
ALT_LIBRARY_DIRS = $(MAKEFILE_DIR)../vbxlibs
ALT_LIBRARY_NAMES = vbxtest fixmath scalar vbxware

INC_DIRS := $(ALT_INCLUDE_DIRS) $(BSP_INC_DIR) $(APP_INCLUDE_DIRS) $(MXP_FRWK_INC_DIR)
LIB_DIRS := $(ALT_LIBRARY_DIRS) $(BSP_LIB_DIR)

INC_DIR_FLAGS := $(addprefix -I,$(INC_DIRS))
LIB_DIR_FLAGS := $(addprefix -L,$(LIB_DIRS))

LIBS := $(addprefix -l, $(ALT_LIBRARY_NAMES)) $(LIBS)

ifndef ELF
ELF := test.elf
endif
ELFSIZE := $(ELF).size
ELFCHECK := $(ELF).elfcheck

###########################################################################

# All Target
all: $(ELF) secondary-outputs

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

.PHONY: print_vars
print_vars:
	@echo "ALT_LIBRARY_ROOT_DIR=$(ALT_LIBRARY_ROOT_DIR)"
	@echo "ALT_LIBRARY_DIRS=$(ALT_LIBRARY_DIRS)"
	@echo "ALT_LIBRARY_NAMES=$(ALT_LIBRARY_NAMES)"
	@echo "ALT_LDDEPS=$(ALT_LDDEPS)"
	@echo "MAKEABLE_LIBRARY_ROOT_DIRS=$(MAKEABLE_LIBRARY_ROOT_DIRS)"
	@echo "LIBS=$(LIBS)"
	@echo "INC_DIRS=$(INC_DIRS)"
	@echo "INC_DIR_FLAGS=$(INC_DIR_FLAGS)"
	@echo "LIB_DIRS=$(LIB_DIRS)"
	@echo "LIB_DIR_FLAGS=$(LIB_DIR_FLAGS)"
	@echo "LIB_TARGETS=$(LIB_TARGETS)"
	@echo "OBJS=$(OBJS)"
	@echo "C_SRCS=$(C_SRCS)"
	@echo "C_DEPS=$(C_DEPS)"
	@echo "MAKEFILE_DIR=$(MAKEFILE_DIR)"
	@echo "HW_PLATFORM_XML=$(HW_PLATFORM_XML)"
	@echo "PROCESSOR_TYPE=$(PROCESSOR_TYPE)"
	@echo "PROCESSOR_INSTANCE=$(PROCESSOR_INSTANCE)"
	@echo "CPU_FLAGS=$(CPU_FLAGS)"
	@echo "Compiler flags=$(CMACRO_DEFS)"


vpath %.c $(sort $(dir $(C_SRCS)))
vpath %.cpp $(sort $(dir $(CXX_SRCS)))

$(OBJ_DIR)/%.o: %.c
	@echo Building file: $<
	@echo Invoking: gcc compiler
	@$(MKDIR) -p $(@D)
	$(CC) $(OPT_FLAGS) $(CC_FLAGS)  $(INC_DIR_FLAGS) $(CPU_FLAGS) -MD  -o"$@" "$<"
	@echo Finished building: $<
	@echo ' '
$(OBJ_DIR)/%.o: %.cpp
	@echo Building file: $<
	@echo Invoking: gcc compiler
	@$(MKDIR) -p $(@D)
	$(CXX) $(OPT_FLAGS) $(CXX_FLAGS)  $(INC_DIR_FLAGS) $(CPU_FLAGS) \
	    -MD  -o"$@" "$<"
	@echo Finished building: $<
	@echo ' '

$(ELF): $(OBJS) $(LD_SCRIPT)
	@echo Building target: $@
	@echo Invoking: gcc linker
	$(LD) $(LD_FLAGS) $(LIB_DIR_FLAGS) $(CPU_FLAGS) -o"$@" $(OBJS) $(LIBS)
	@echo Finished building target: $@
	@echo ' '

$(ELFSIZE): $(ELF)
	@echo Invoking: Print Size
	$(SZ) "$<" | tee "$@"
	@echo Finished building: $@
	@echo ' '

$(ELFCHECK): $(ELF)
	@echo Invoking: Xilinx ELF Check
	elfcheck "$<" -hw $(HW_PLATFORM_XML) -pe $(PROCESSOR_INSTANCE) | tee "$@"
	@echo Finished building: $@
	@echo ' '

# Load FPGA bitstream
.PHONY: pgm
ifeq ($(PROCESSOR_TYPE), microblaze)
pgm:
	cd $(PROJ_ROOT) && xmd -tcl xmd_init.tcl
else
pgm:
	cd $(PROJ_ROOT) && xmd -tcl xmd_init.tcl
endif

IP_ADDRESS=
# Download ELF and execute
.PHONY: run
ifeq ($(OS_TARGET),LINUX)
run: $(ELF)
	scp $(ELF) "root@$(IP_ADDRESS):."  && ssh root@$(IP_ADDRESS) ./$(ELF)
else
ifeq ($(PROCESSOR_TYPE), microblaze)
run: $(ELF)
	xmd -tcl $(MAKEFILE_DIR)/xmd_mb.tcl
else
run: $(ELF)
	xmd -tcl $(MAKEFILE_DIR)/xmd_arm.tcl
endif
endif
# Other Targets
clean:
	-$(RM) $(OBJS) $(C_DEPS) $(ELFSIZE) $(ELFCHECK) $(ELF)
	-$(RM) $(OBJ_ROOT_DIR)
	-@echo ' '

# $(ELFCHECK) removed because elfcheck does not exist in Vivado > 2013.4
secondary-outputs: $(ELFSIZE)

.PHONY: all clean
.SECONDARY:

###########################################################################



