#ifndef COMMON_H
#define COMMON_H

#define data_t uint8_t //for MNIST grayscale images 
#define BOOL uint8_t
#define ERR_TOL 0.001

#define NUM_PES 16 // number of PEs in system
#define SIZE_IMAGE (28*28) //MNIST 28x28
#define IMG_WIDTH 28
#define L1_KERNEL_WIDTH 7
#define L1_KERNEL_SIZE (7*7)
#define L2_KERNEL_WIDTH 5
#define L2_KERNEL_SIZE (5*5)
#define L1_MAX_MAPS 4
#define L2_MAX_MAPS 8
#define L1_MAP_SIZE (11*11)
#define L2_MAP_SIZE (4*4)
#define L1_TO_L2 10
#define UNROLL_FACTOR 5 
#define L1_MAPS 50
#define L2_MAPS 128
#define DOWN_FAC1 2
#define DOWN_FAC2 2

#define BASE_ADDR 0x4008
#define L1_KERNEL_ADDR 0x4318
#define L2_KERNEL_ADDR 0x43ec
#define L1_KERNEL_SCALE_ADDR 0x43dc
#define L2_KERNEL_SCALE_ADDR 0x44b4
#define L1_MAPS_ADDR 0x44d4
#define L2_MAPS_ADDR 0x489c
#define L2_L1_MAP_ADDR 0x499c
#define L2_L1_MAP_STATE_ADDR 0x4adc
#define DONE_ADDR 0x4f98
#define SCRATCHPAD_ADDR 0x4fa8

#endif
