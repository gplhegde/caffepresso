#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "../../types.h"
#include "../../parameters.h"
#define NUM_PARAMS 17

#define BASE_ADDR 0x3400 //base address to start loading data at - might have to explore manually
unsigned FREE_SPACE = 0x7800 - BASE_ADDR; //let's keep stack size fixed at 2kB
unsigned curr_address = BASE_ADDR;

unsigned D_FREE_SPACE = 0x1fffffff - 0x1f000000;
unsigned d_curr_address = 0x1f000000;

unsigned e_malloc(unsigned size, unsigned align);
unsigned d_malloc(unsigned size, unsigned align);
ADDR_T e_get_global_address(unsigned coreid, unsigned local_addr);

int main (int argc, char **argv) {

	int i,j,k;
	
	unsigned num_maps[NUM_LAYERS] = NUM_MAPS;
	unsigned map_widths[NUM_LAYERS] = MAP_WIDTH;
	unsigned map_heights[NUM_LAYERS] = MAP_HEIGHT;
	unsigned patch_widths[NUM_LAYERS] = PATCH_WIDTH;
	unsigned patch_heights[NUM_LAYERS] = PATCH_HEIGHT;
	unsigned subsampling_factors[NUM_LAYERS] = SUBSAMPLING_FACTORS;
	unsigned window_sizes[NUM_LAYERS] = WINDOW_SIZES;
	unsigned kernel_widths[NUM_LAYERS] = KERNEL_WIDTH; //NOTE: kernels are always square sizes.
	unsigned conv_strides[NUM_LAYERS] = CONV_STRIDES;
	unsigned conv_pads[NUM_LAYERS] = CONV_PAD;
	unsigned pooling_strides[NUM_LAYERS] = POOLING_STRIDES;

	unsigned patch_width_from_image = patch_widths[0]*subsampling_factors[0]*conv_strides[0] + kernel_widths[0] - 1;
	unsigned patch_height_from_image = patch_heights[0]*subsampling_factors[0]*conv_strides[0] + kernel_widths[0] - 1;
	printf("patch width from image = %d, patch height from image = %d\n",patch_width_from_image,patch_height_from_image);
	unsigned patch_size_from_image = patch_width_from_image*patch_height_from_image;
	unsigned num_patches[NUM_LAYERS];
	unsigned patch_width_from_maps[NUM_LAYERS-1];
	unsigned patch_height_from_maps[NUM_LAYERS-1];
	for (i=0;i<NUM_LAYERS;i++){
		if (i==0){
				num_patches[i] = map_widths[i]/patch_widths[i];
				num_patches[i] = num_patches[i]*map_heights[i]/patch_heights[i];
				printf("Patches from image = %d\n",num_patches[i]);
		} else {
				num_patches[i] = ((map_widths[i-1] - kernel_widths[i] + 1)/subsampling_factors[i])/patch_widths[i];	
				num_patches[i] = num_patches[i]*((map_heights[i-1] - kernel_widths[i] + 1)/subsampling_factors[i])/patch_heights[i];	
				printf("Patches from layer %d = %d\n",i,num_patches[i]);
				patch_width_from_maps[i-1] = patch_widths[i]*subsampling_factors[i] + kernel_widths[i] - 1; 
				patch_height_from_maps[i-1] = patch_heights[i]*subsampling_factors[i] + kernel_widths[i] - 1; 
		}
	}
	/**************** DRAM MALLOCS *******************/
	//store flattened image
	unsigned d_image = d_malloc(num_patches[0]*patch_size_from_image*sizeof(IMAGE_T),sizeof(IMAGE_T));
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
	unsigned maps[NUM_LAYERS];
	for (i=0;i<NUM_LAYERS;i++){
			if (i==NUM_LAYERS-1){//last layer maps do not need to be flattened..
				maps[i] = d_malloc(num_maps[i]*map_widths[i]*map_heights[i]*sizeof(MAP_T),sizeof(MAP_T));
				printf("Layer %d maps stored at  0x%x\n",i+1,maps[i]);
			} else {
				maps[i] = d_malloc(num_maps[i]*num_patches[i+1]*patch_width_from_maps[i]*patch_height_from_maps[i]*sizeof(MAP_T),sizeof(MAP_T));
				printf("Layer %d maps stored at  0x%x\n",i+1,maps[i]);
			}
	}

	/****************** ECORE MALLOCS ****************/
	
	unsigned mailbox = e_malloc(4*sizeof(unsigned),sizeof(unsigned));
	unsigned globals = e_malloc(NUM_PARAMS*sizeof(GLOBAL_CONSTANTS_T),sizeof(GLOBAL_CONSTANTS_T));
	unsigned window_scale = e_malloc(sizeof(SCALE_T),sizeof(SCALE_T));
	
	printf("\n");

	printf("#define DRAM_KERNEL_OFFSETS {%d,%d}\n",(l1_kernel-d_image),(l2_kernel-d_image));
	printf("#define DRAM_KERNEL_SCALE_OFFSETS {%d,%d}\n",(l1_scale-d_image),(l2_scale-d_image));
	printf("#define E_DRAM_KERNEL_PTRS {0x%x,0x%x}\n",(l1_kernel+0x70000000),(l2_kernel+0x70000000));
	printf("#define E_DRAM_PATCH_PTRS {0x%x,",(d_image+0x70000000));
	for (i=0;i<NUM_LAYERS-1;i++){
		printf("0x%x,",maps[i]+0x70000000);
	}
	printf("}\n");
	printf("#define E_DRAM_MAP_PTRS {");
	for (i=0;i<NUM_LAYERS;i++){
		printf("0x%x,",maps[i]+0x70000000);
	}
	printf("}\n");
	printf("#define DRAM_TOTAL_SIZE %d\n",d_curr_address-d_image);
	printf("#define DRAM_INTERMEDIATE_MAP_OFFSETS {");
	for (i=0;i<NUM_LAYERS;i++){
		printf("%d,",maps[i]-d_image);
	}
	printf("}\n");
	//printf("#define DRAM_PARAMETERS_ADDR 0x%x\n",params);
	//printf("#define E_DRAM_PARAMETERS_ADDR 0x%x\n",(params+0x70000000));
	//printf("#define DRAM_IMAGE_ADDR 0x%x\n",d_image);
	//printf("#define E_DRAM_IMAGE_ADDR 0x%x\n",(d_image+0x70000000));
	//printf("#define DRAM_INTERMEDIATE_MAP_ADDR 0x%x\n",l1_map);
	//printf("#define E_DRAM_INTERMEDIATE_MAP_ADDR 0x%x\n",(l1_map+0x70000000));
	printf("#define PARAMETERS_ADDR 0x%x\n",globals);
	printf("#define WINDOW_SCALE_ADDR 0x%x\n",window_scale);
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

