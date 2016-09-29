#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define NUM_PES 16
#define L1_MAPS 32
#define L2_MAPS 32
#define L3_MAPS 64
#define L1_MAPS_PER_ECORE 2
#define L2_MAPS_PER_ECORE 2
#define L3_MAPS_PER_ECORE 4
#define L1_KERNEL_WIDTH 5
#define L2_KERNEL_WIDTH 5
#define L3_KERNEL_WIDTH 5
#define L1_KERNEL_SIZE (5*5)
#define L2_KERNEL_SIZE (5*5)
#define L3_KERNEL_SIZE (5*5)
#define L1_MAP_SIZE (16*16) //downsample by 2
#define L2_MAP_SIZE (8*8) //downsample by 2
#define L3_MAP_SIZE (4*4) //downsample by 2
#define L1_MAP_WIDTH 16
#define L2_MAP_WIDTH 8
#define L3_MAP_WIDTH 4
#define IMAGE_SIZE (36*36)
#define IMAGE_WIDTH 36
#define L1_MAP_UNROLL_FACTOR 1
#define L2_MAP_UNROLL_FACTOR 1
#define L3_MAP_UNROLL_FACTOR 1
#define DOWN_FAC1 2
#define DOWN_FAC2 2
#define DOWN_FAC3 2
#define STATE_BUFFERS 4

#define BASE_ADDR 0x3200 //base address to start loading data at - might have to explore manually

//define types for data structures
#define GLOBAL_CONSTANTS_T unsigned
#define L1_KERNEL_T float
#define L2_KERNEL_T float
#define L3_KERNEL_T float
#define L1_MAP_T float
#define L2_MAP_T float //L2 map must be float -- for classification
#define L3_MAP_T float
#define IMAGE_T float
#define MEM_ADDR_T unsigned //memory address type
#define SCALE_T float
#define INTERMEDIATE_T float //data type for storing intermediate values when operating each map

#ifdef HIGH_PRECISION
	#define GLOBAL_CONSTANTS_T unsigned
	#define L1_KERNEL_T float
	#define L2_KERNEL_T float
	#define L3_KERNEL_T float
	#define L1_MAP_T float
	#define L2_MAP_T float //L2 map must be float -- for classification
	#define L3_MAP_T float
	#define IMAGE_T float
	#define MEM_ADDR_T unsigned //memory address type
	#define SCALE_T float
	#define INTERMEDIATE_T float //data type for storing intermediate values when operating each map
#endif

unsigned FREE_SPACE = 0x7800 - BASE_ADDR; //let's keep stack size fixed at 2kB
unsigned curr_address = BASE_ADDR;

unsigned e_malloc(unsigned size, unsigned align);
MEM_ADDR_T e_get_global_address(unsigned coreid, unsigned local_addr);

int main (int argc, char **argv) {

	//4 global constants per eCore -- timesteps, L1_num_maps & L2_num_maps & L3_num_maps
	printf("Allocating space for constants..\n");
	unsigned globals = e_malloc(4*sizeof(GLOBAL_CONSTANTS_T),sizeof(GLOBAL_CONSTANTS_T)); 
	printf("Allocating space for image..\n");
	unsigned image = e_malloc(IMAGE_SIZE*sizeof(IMAGE_T),sizeof(IMAGE_T));
	printf("Allocating space for L1_kernel..\n");
	unsigned L1_kernel = e_malloc(L1_KERNEL_SIZE*L1_MAPS_PER_ECORE*sizeof(L1_KERNEL_T),sizeof(L1_KERNEL_T));
	printf("Allocating space for L1_kernel_scale..\n");
	unsigned L1_kernel_scale = e_malloc(L1_MAPS_PER_ECORE*sizeof(SCALE_T),sizeof(SCALE_T)); 
	printf("Allocating space for L2_kernel..\n");
	unsigned L2_kernel = e_malloc(L2_KERNEL_SIZE*L2_MAPS_PER_ECORE*sizeof(L2_KERNEL_T),sizeof(L2_KERNEL_T));
	printf("Allocating space for L2_kernel_scale..\n");
	unsigned L2_kernel_scale = e_malloc(L2_MAPS_PER_ECORE*sizeof(SCALE_T),sizeof(SCALE_T)); 
	printf("Allocating space for L3_kernel..\n");
	unsigned L3_kernel = e_malloc(L3_KERNEL_SIZE*L3_MAPS_PER_ECORE*sizeof(L3_KERNEL_T),sizeof(L3_KERNEL_T));
	printf("Allocating space for L3_kernel_scale..\n");
	unsigned L3_kernel_scale = e_malloc(L3_MAPS_PER_ECORE*sizeof(SCALE_T),sizeof(SCALE_T)); 
	printf("Allocating space for L3_maps..\n");
	unsigned L3_maps = e_malloc(L3_MAP_SIZE*L3_MAPS_PER_ECORE*sizeof(L3_MAP_T),sizeof(L3_MAP_T));
	printf("Allocating space for local accums..\n");
	unsigned local_accum_addr = e_malloc(L1_MAP_SIZE*sizeof(L1_MAP_T),sizeof(L1_MAP_T)); //largest map accum would be of L1
	printf("Allocating space for local accum mem read states..\n");
	unsigned accum_map_state = e_malloc(STATE_BUFFERS*L1_MAP_SIZE*sizeof(L1_MAP_T),sizeof(L1_MAP_T)); //largest map = L1
	printf("Allocating space for mailboxes..\n");
	unsigned mailbox = e_malloc(4*sizeof(unsigned),sizeof(unsigned));

	//let's work out scratchpad usage
	//inside L1
	unsigned scratch_s = 0;
	unsigned filter2D_out_size = IMAGE_SIZE*sizeof(INTERMEDIATE_T);
	scratch_s += L1_MAP_UNROLL_FACTOR*filter2D_out_size;

	//inside L2
	unsigned temp = 0;
	temp += L2_MAP_UNROLL_FACTOR*L2_MAP_SIZE*sizeof(INTERMEDIATE_T);//number of filter2D_out buffers
	temp += (L1_MAP_WIDTH+4)*(L1_MAP_WIDTH+4)*sizeof(L1_MAP_T);//accum vector
	
	if (temp > scratch_s)
		scratch_s = temp;

	//inside L3
	temp = 0;
	temp += L3_MAP_UNROLL_FACTOR*L3_MAP_SIZE*sizeof(INTERMEDIATE_T);//number of filter2D_out buffers
	temp += (L2_MAP_WIDTH+4)*(L2_MAP_WIDTH+4)*sizeof(L2_MAP_T);//accum vector

	if (temp > scratch_s)
		scratch_s = temp;

	printf("Allocating space for L3_kernel_scale..\n");
	unsigned scratchpad = e_malloc(scratch_s,sizeof(INTERMEDIATE_T));//let's keep it aligned to intermediate type 
	unsigned scratchpad_end = scratchpad + scratch_s;

	/*
	printf("L1_kernel = 0x%x, L2_kernel = 0x%x\n",L1_kernel,L2_kernel);
	printf("L1_kernel_scale = 0x%x, L2_kernel_scale = 0x%x\n",L1_kernel_scale,L2_kernel_scale);
	printf("L1_maps = 0x%x, L2_maps = 0x%x\n",L1_maps,L2_maps);
	printf("l1map_addr = 0x%x, l1map_state = 0x%x\n",l1map_addr,l1map_state);
	printf("Mailboxes = 0x%x, scratchpad = 0x%x\n",mailbox,scratchpad);
	printf("SCRATCHPAD SIZE = %d bytes\n",scratch_s);
	printf("Scratchpad End = 0x%x\n",scratchpad_end);
	printf("Available space = %d bytes\n",FREE_SPACE);
	printf("Stack space = 2kB\n");
	*/

	//success if you got this far..
	printf("\n");
	printf("SUCCESS\n");
	printf("\n");
	
	printf("FREE SPACE=%d\n\n",FREE_SPACE);

	printf("#define GLOBAL_CONSTANTS_ADDR 0x%x\n",globals);
	printf("#define IMAGE_ADDR 0x%x\n",image);
	printf("#define L1_KERNEL_ADDR 0x%x\n",L1_kernel);
	printf("#define L2_KERNEL_ADDR 0x%x\n",L2_kernel);
	printf("#define L3_KERNEL_ADDR 0x%x\n",L3_kernel);
	printf("#define L1_KERNEL_SCALE_ADDR 0x%x\n",L1_kernel_scale);
	printf("#define L2_KERNEL_SCALE_ADDR 0x%x\n",L2_kernel_scale);
	printf("#define L3_KERNEL_SCALE_ADDR 0x%x\n",L3_kernel_scale);
	printf("#define L3_MAPS_ADDR 0x%x\n",L3_maps);
	printf("#define LOCAL_ACCUM_ADDR 0x%x\n",local_accum_addr);
	printf("#define ACCUM_MAPS_STATE_ADDR 0x%x\n",accum_map_state);
	printf("#define DONE_ADDR 0x%x\n",mailbox);
	printf("#define SCRATCHPAD_ADDR 0x%x\n",scratchpad);
	int i;
	printf("#define LOCAL_ACCUM_ECORE_ADDRS {");
	for (i=0;i<NUM_PES;i++){
		unsigned gaddress = e_get_global_address(i,local_accum_addr);
		printf("0x%x,",gaddress);
	}
	printf("}\n");

}

MEM_ADDR_T e_get_global_address(unsigned coreid, unsigned local_addr){
	
	int col = coreid%4;
	int row = (coreid-col)/4;

	unsigned address = 0x80800000 + row*(0x4000000) + col*(0x100000) + local_addr;

	return address;
}

MEM_ADDR_T e_malloc(unsigned size, unsigned align){

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

