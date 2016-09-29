#define NUM_PES 16 // number of PEs in system
#define NUM_LAYERS 2
#define IMAGE_SIZE (151*151)
#define IMAGE_WIDTH 151
#define IMAGE_HEIGHT 151
#define L1_KERNEL_WIDTH 9
#define L1_KERNEL_SIZE (9*9)
#define L2_KERNEL_WIDTH 9
#define L2_KERNEL_SIZE (9*9)
#define L1_MAX_MAPS_PER_ECORE 4
#define L2_MAX_MAPS_PER_ECORE 16
#define L1_MAP_SIZE (28*28)
#define L1_MAP_WIDTH 28
#define L1_MAP_HEIGHT 28
#define L2_MAP_SIZE (5*5)
#define L2_MAP_WIDTH 5
#define L2_MAP_HEIGHT 5
#define L1_MAPS 64
#define L2_MAPS 256 
#define WINDOW_SIZE1 10
#define WINDOW_SIZE2 6
#define DOWN_FAC1 5
#define DOWN_FAC2 4


#define NUM_MAPS {64,256}
#define MAP_WIDTH {28,5}
#define MAP_HEIGHT {28,5}
#define PATCH_WIDTH {4,5}
#define PATCH_HEIGHT {4,5}
#define SUBSAMPLING_FACTORS {5,4}
#define WINDOW_SIZES {10,6}
#define KERNEL_WIDTH {9,9}
