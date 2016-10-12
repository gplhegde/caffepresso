# Get directory where this file resides
MAKEFILE_DIR := $(dir $(lastword $(MAKEFILE_LIST)))

include $(MAKEFILE_DIR)/xil_vars.mk

ifndef INC_DIRS
INC_DIRS := $(BSP_INC_DIR)
endif
INC_DIR_FLAGS := $(addprefix -I,$(INC_DIRS))

###########################################################################

all: $(OUTPUT_LIB)

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

$(OBJ_DIR)/%.o: %.c
	@echo Building file: $<
	@echo Invoking: gcc compiler
	@$(MKDIR) -p $(@D)
	$(CC) $(CC_FLAGS) $(OPT_FLAGS) $(INC_DIR_FLAGS) $(CPU_FLAGS) \
	    -MD -o"$@" "$<"
	@echo Finished building: $<
	@echo ' '

$(OBJ_DIR)/%.o: %.cpp
	@echo Building file: $<
	@echo Invoking: gcc compiler
	@$(MKDIR) -p $(@D)
	$(CXX) $(CXX_FLAGS) $(OPT_FLAGS) $(INC_DIR_FLAGS) $(CPU_FLAGS) \
	    -MD  -o"$@" "$<"
	@echo Finished building: $<
	@echo ' '


$(OUTPUT_LIB): $(OBJS)
	@echo Building target: $@
	@echo Invoking: archiver
	$(AR) -r "$@" $(OBJS)
	@echo Finished building target: $@
	@echo ' '

clean:
	-$(RM) $(OBJS) $(C_DEPS) $(OUTPUT_LIB)
	-$(RM) $(OBJ_ROOT_DIR)
	-@echo ' '

.PHONY: all clean $(OUTPUT_LIB)
.SECONDARY:
