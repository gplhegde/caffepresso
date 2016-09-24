ifdef MXP_SIM
include sim/sim_srcs.mk
endif
C_SRCS+=main.c
C_SRCS+=$(wildcard $(MXP_FRWK_SRC_DIR)/*.c)
C_SRCS+=$(wildcard $(TEST_SRC_DIR)/*.c)

