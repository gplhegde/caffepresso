#define NUM_PES 16 // number of PEs in system
#define NUM_LAYERS 2
#define IMAGE_SIZE (96*96)
#define IMAGE_HEIGHT 96
#define IMAGE_WIDTH 96
#define L1_KERNEL_WIDTH 5
#define L1_KERNEL_HEIGHT 5
#define L1_KERNEL_SIZE (5*5)
#define L2_KERNEL_WIDTH 5
#define L2_KERNEL_HEIGHT 5
#define L2_KERNEL_SIZE (5*5)
#define L1_MAX_MAPS_PER_ECORE 8
#define L2_MAX_MAPS_PER_ECORE 4
#define L1_MAP_SIZE (46*46)
#define L1_MAP_HEIGHT 46
#define L1_MAP_WIDTH 46
#define L2_MAP_SIZE (21*21)
#define L2_MAP_HEIGHT 21
#define L2_MAP_WIDTH 21
#define L1_MAPS 128
#define L2_MAPS 64
#define DOWN_FAC1 2
#define DOWN_FAC2 2


#define NUM_MAPS {128,64}
#define MAP_HEIGHT {46,21}
#define MAP_WIDTH {46,21}
#define PATCH_HEIGHT {46,21}
#define PATCH_WIDTH {2,7}
#define SUBSAMPLING_FACTORS {2,2}
#define KERNEL_WIDTH {5,5}
