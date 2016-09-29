#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "parameters.h"

#define BASE_ADDR 0x4000 //base address to start loading data at - might have to explore manually

//define types for data structures
#define GLOBAL_CONSTANTS_T unsigned
#define L1_KERNEL_T float
#define L2_KERNEL_T float
#define L3_KERNEL_T float
#define L1_MAP_T float
#define L2_MAP_T float
#define L3_MAP_T float
#define IMAGE_T float
#define MEM_ADDR_T unsigned //memory address type
#define SCALE_T float
#define INTERMEDIATE_T float //data type for storing intermediate values when operating each map

#define L1_MAP_UNROLL_FACTOR 1
#define L2_MAP_UNROLL_FACTOR 2
#define L3_MAP_UNROLL_FACTOR 2
#define NUM_BUFFERS 2

unsigned FREE_SPACE = 0x7800 - BASE_ADDR; //let's keep stack size fixed at 2kB
unsigned curr_address = BASE_ADDR;

unsigned e_malloc(unsigned size, unsigned align);
MEM_ADDR_T e_get_global_address(unsigned coreid, unsigned local_addr);

int main (int argc, char **argv) {

	int i;

	//1 global constant per eCore -- timesteps.. 
	printf("Assigning memory for global constants...\n");
	unsigned gc = e_malloc(sizeof(GLOBAL_CONSTANTS_T),sizeof(GLOBAL_CONSTANTS_T));
	printf("Assigning memory for image...\n");
	unsigned image = e_malloc(IMAGE_SIZE*sizeof(IMAGE_T),sizeof(IMAGE_T));
	printf("Assigning memory for L1 kernel...\n");
	unsigned L1_kernel = e_malloc(L1_KERNEL_SIZE*L1_MAX_MAPS_PER_ECORE*sizeof(L1_KERNEL_T),sizeof(L1_KERNEL_T));
	printf("Assigning memory for L1 scale...\n");
	unsigned L1_kernel_scale = e_malloc(L1_MAX_MAPS_PER_ECORE*sizeof(SCALE_T),sizeof(SCALE_T)); 
	printf("Assigning memory for L2 kernel...\n");
	unsigned L2_kernel = e_malloc(L2_KERNEL_SIZE*L2_MAX_MAPS_PER_ECORE*sizeof(L2_KERNEL_T),sizeof(L2_KERNEL_T));
	printf("Assigning memory for L2 scale...\n");
	unsigned L2_kernel_scale = e_malloc(L2_MAX_MAPS_PER_ECORE*sizeof(SCALE_T),sizeof(SCALE_T)); 
	printf("Assigning memory for L3 kernel...\n");
	unsigned L3_kernel = e_malloc(L3_KERNEL_SIZE*L3_MAX_MAPS_PER_ECORE*sizeof(L3_KERNEL_T),sizeof(L3_KERNEL_T));
	printf("Assigning memory for L3 scale...\n");
	unsigned L3_kernel_scale = e_malloc(L3_MAX_MAPS_PER_ECORE*sizeof(SCALE_T),sizeof(SCALE_T)); 
	printf("Assigning memory for mailbox...\n");
	unsigned mailbox = e_malloc(4*sizeof(unsigned),sizeof(unsigned));
	printf("Assigning memory for maps...\n");
	unsigned maps = e_malloc(L1_MAP_SIZE*L1_MAX_MAPS_PER_ECORE*sizeof(L1_MAP_T),sizeof(L1_MAP_T));
	printf("Assigning memory for scratchpad...\n");

	//let's work out scratchpad usage
	//inside L1
	unsigned scratch_s = 0;
	unsigned conv_size = IMAGE_WIDTH - L1_KERNEL_WIDTH + 1;
	unsigned filter2D_out_size = conv_size*conv_size*sizeof(INTERMEDIATE_T);
	scratch_s += L1_MAP_UNROLL_FACTOR*filter2D_out_size;

	//inside L2
	unsigned temp = 0;
	conv_size = L1_MAP_WIDTH; //conv_size has decreased
	temp += L1_MAP_SIZE*L1_MAX_MAPS_PER_ECORE*sizeof(L1_MAP_T); //dest0
	temp += (L1_MAP_WIDTH*L1_MAP_WIDTH)*L1_MAX_MAPS_PER_ECORE*sizeof(L1_MAP_T); //dest_pad
	temp += L2_MAP_UNROLL_FACTOR*conv_size*conv_size*sizeof(INTERMEDIATE_T);//number of filter2D_out buffers
	
	if (temp > scratch_s)
		scratch_s = temp;

	//inside L3
	temp = 0;
	conv_size = L3_MAP_WIDTH; //conv_size has decreased
	temp += L2_MAP_SIZE*L2_MAX_MAPS_PER_ECORE*sizeof(L2_MAP_T); //dest0
	temp += (L2_MAP_WIDTH*L2_MAP_WIDTH)*L2_MAX_MAPS_PER_ECORE*sizeof(L2_MAP_T); //dest_pad
	temp += L3_MAP_UNROLL_FACTOR*conv_size*conv_size*sizeof(INTERMEDIATE_T);//number of filter2D_out buffers
	
	if (temp > scratch_s)
		scratch_s = temp;

	unsigned scratchpad = e_malloc(scratch_s,sizeof(INTERMEDIATE_T));//let's keep it aligned to intermediate type 
	unsigned scratchpad_end = scratchpad + scratch_s;

	//success if you got this far..
	printf("\n");
	printf("SUCCESS\n");
	printf("\n");
	
	printf("FREE SPACE=%d\n\n",FREE_SPACE);

	printf("#define GLOBAL_CONSTANTS_ADDR 0x%x\n",gc);
	printf("#define IMAGE_ADDR 0x%x\n",image);
	printf("#define L1_KERNEL_ADDR 0x%x\n",L1_kernel);
	printf("#define L2_KERNEL_ADDR 0x%x\n",L2_kernel);
	printf("#define L3_KERNEL_ADDR 0x%x\n",L3_kernel);
	printf("#define L1_KERNEL_SCALE_ADDR 0x%x\n",L1_kernel_scale);
	printf("#define L2_KERNEL_SCALE_ADDR 0x%x\n",L2_kernel_scale);
	printf("#define L3_KERNEL_SCALE_ADDR 0x%x\n",L3_kernel_scale);
	printf("#define DONE_ADDR 0x%x\n",mailbox);
	printf("#define MAPS_ADDR 0x%x\n",maps);
	printf("#define MAPS_ECORE_ADDRS {");
	for (i=0;i<NUM_PES;i++){
		unsigned gaddress = e_get_global_address(i,maps);
		printf("0x%x,",gaddress);
	}
	printf("}\n");
}

unsigned e_malloc(unsigned size, unsigned align){

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
	unsigned old_free_space = FREE_SPACE;
	FREE_SPACE -= size;
		
//	printf("FREE_SPACE = %d, REQUESTED SPACE = %d, NEW_FREE_SPACE = %d\n",old_free_space,size,FREE_SPACE);

	return res;
}

MEM_ADDR_T e_get_global_address(unsigned coreid, unsigned local_addr){
	
	int col = coreid%4;
	int row = (coreid-col)/4;

	unsigned address = 0x80800000 + row*(0x4000000) + col*(0x100000) + local_addr;

	return address;
}
