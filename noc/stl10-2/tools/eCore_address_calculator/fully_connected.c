#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define NUM_PARAMS 10
#define NUM_PES 16
#define L1_MAPS 128
#define L2_MAPS 64
#define L1_MAPS_PER_ECORE 8
#define L2_MAPS_PER_ECORE 4
#define L1_KERNEL_WIDTH 5
#define L2_KERNEL_WIDTH 5
#define L1_KERNEL_SIZE (5*5)
#define L2_KERNEL_SIZE (5*5)
#define L1_MAP_SIZE (46*46) //downsample by 2
#define L2_MAP_SIZE (21*21) //downsample by 2
#define IMAGE_SIZE (96*96)
#define IMAGE_HEIGHT 96
#define IMAGE_WIDTH 96
#define DOWN_FAC1 2
#define DOWN_FAC2 2
#define STATE_BUFFERS 4
#define NUM_PATCHES (1*23)
#define NUM_PATCHES_L1 (1*3)
#define PATCH_FROM_IMAGE_SIZE (96*8)
#define PATCH_FROM_L1_SIZE (46*18)
#define NUM_LAYERS 2

#define BASE_ADDR 0x3000 //base address to start loading data at - might have to explore manually

//define types for data structures
#define GLOBAL_CONSTANTS_T unsigned
#define KERNEL_T float
#define MAP_T float
#define IMAGE_T float
#define PATCH_T float
#define ADDR_T unsigned //memory address type
#define SCALE_T float
#define INTERMEDIATE_T float //data type for storing intermediate values when operating each map

unsigned FREE_SPACE = 0x7800 - BASE_ADDR; //let's keep stack size fixed at 2kB
unsigned curr_address = BASE_ADDR;

unsigned D_FREE_SPACE = 0x1fffffff - 0x1f000000;
unsigned d_curr_address = 0x1f000000;

unsigned e_malloc(unsigned size, unsigned align);
unsigned d_malloc(unsigned size, unsigned align);
ADDR_T e_get_global_address(unsigned coreid, unsigned local_addr);

int main (int argc, char **argv) {

	int i,j,k;

	/**************** DRAM MALLOCS *******************/
	//store flattened image
	unsigned d_image = d_malloc(NUM_PATCHES*PATCH_FROM_IMAGE_SIZE*sizeof(IMAGE_T),sizeof(IMAGE_T));
	printf("image address on dram = 0x%x\n",d_image);
	//unsigned params = d_malloc(NUM_LAYERS*9*sizeof(GLOBAL_CONSTANTS_T),sizeof(GLOBAL_CONSTANTS_T));
	//printf("parameters stored on dram = 0x%x\n",params);
	unsigned l1_kernel = d_malloc(L1_KERNEL_SIZE*L1_MAPS*sizeof(KERNEL_T),sizeof(KERNEL_T));
	printf("l1 kernels stored on dram = 0x%x\n",l1_kernel);
	unsigned l1_scale = d_malloc(L1_MAPS*sizeof(SCALE_T),sizeof(SCALE_T));
	printf("l1 scales stored on dram = 0x%x\n",l1_scale);
	unsigned l2_kernel = d_malloc(L2_KERNEL_SIZE*L2_MAPS*sizeof(KERNEL_T),sizeof(KERNEL_T));
	printf("l2 kernels stored on dram = 0x%x\n",l2_kernel);
	unsigned l2_scale = d_malloc(L2_MAPS*sizeof(SCALE_T),sizeof(SCALE_T));
	printf("l2 scales stored on dram = 0x%x\n",l2_scale);
	unsigned l1_map = d_malloc(NUM_PATCHES_L1*PATCH_FROM_L1_SIZE*sizeof(MAP_T),sizeof(MAP_T));
	printf("l1 map stored at  0x%x\n",l1_map);

	/****************** ECORE MALLOCS ****************/
	
	unsigned mailbox = e_malloc(4*sizeof(unsigned),sizeof(unsigned));
	unsigned globals = e_malloc(NUM_PARAMS*sizeof(GLOBAL_CONSTANTS_T),sizeof(GLOBAL_CONSTANTS_T));
	
	printf("\n");

	printf("#define DRAM_KERNEL_OFFSETS {%d,%d}\n",(l1_kernel-d_image),(l2_kernel-d_image));
	printf("#define DRAM_KERNEL_SCALE_OFFSETS {%d,%d}\n",(l1_scale-d_image),(l2_scale-d_image));
	printf("#define E_DRAM_KERNEL_PTRS {0x%x,0x%x}\n",(l1_kernel+0x70000000),(l2_kernel+0x70000000));
	printf("#define E_DRAM_PATCH_PTRS {0x%x,0x%x}\n",(d_image+0x70000000),(l1_map+0x70000000));
	printf("#define E_DRAM_MAP_PTRS {0x%x,0x%x}\n",(l1_map+0x70000000),(l1_map+0x70000000));
	printf("#define DRAM_TOTAL_SIZE %d\n",d_curr_address-d_image);
	printf("#define DRAM_INTERMEDIATE_MAP_OFFSET %d\n",l1_map-d_image);
	//printf("#define DRAM_PARAMETERS_ADDR 0x%x\n",params);
	//printf("#define E_DRAM_PARAMETERS_ADDR 0x%x\n",(params+0x70000000));
	//printf("#define DRAM_IMAGE_ADDR 0x%x\n",d_image);
	//printf("#define E_DRAM_IMAGE_ADDR 0x%x\n",(d_image+0x70000000));
	//printf("#define DRAM_INTERMEDIATE_MAP_ADDR 0x%x\n",l1_map);
	//printf("#define E_DRAM_INTERMEDIATE_MAP_ADDR 0x%x\n",(l1_map+0x70000000));
	printf("#define PARAMETERS_ADDR 0x%x\n",globals);
	printf("#define DONE_ADDR 0x%x\n",mailbox);

	//success if you got this far..
	printf("\n");
	printf("SUCCESS\n");
	printf("\n");
	
	printf("FREE SPACE=%d bytes\n\n",FREE_SPACE);
	printf("DRAM FREE SPACE=%d bytes\n\n",D_FREE_SPACE);
}

ADDR_T e_get_global_address(unsigned coreid, unsigned local_addr){
	
	int col = coreid%4;
	int row = (coreid-col)/4;

	unsigned address = 0x80800000 + row*(0x4000000) + col*(0x100000) + local_addr;

	return address;
}

ADDR_T e_malloc(unsigned size, unsigned align){

	if(size==0) {
		printf("Why are you attempting to allocate a 0-sized memory object? \n");
		exit(1);
	}
	if(size>FREE_SPACE) {
		printf("Out of local memory on the eCore\n"); 
		exit(1);
	}
	
	unsigned res = curr_address;
	if (curr_address%align != 0){
		curr_address += align - (curr_address%align);
		FREE_SPACE -= (align - (curr_address%align));
		res = curr_address;
	}
	curr_address += size;
	FREE_SPACE -= size;
		
//	printf("FREE_SPACE = %d, REQUESTED SPACE = %d, NEW_FREE_SPACE = %d\n",old_free_space,size,FREE_SPACE);

	return res;
}

ADDR_T d_malloc(unsigned size, unsigned align){

	if(size==0) {
		printf("Why are you attempting to allocate a 0-sized memory object? \n");
		exit(1);
	}
	if(size>D_FREE_SPACE) {
		printf("Out of DRAM memory on the eCore\n"); 
		exit(1);
	}
	
	unsigned res = d_curr_address;
	if (d_curr_address%align != 0){
		d_curr_address += align - (d_curr_address%align);
		D_FREE_SPACE -= (align - (d_curr_address%align));
		res = d_curr_address;
	}
	d_curr_address += size;
	D_FREE_SPACE -= size;
		
//	printf("FREE_SPACE = %d, REQUESTED SPACE = %d, NEW_FREE_SPACE = %d\n",old_free_space,size,FREE_SPACE);

	return res;
}

