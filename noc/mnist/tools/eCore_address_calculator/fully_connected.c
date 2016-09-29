#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/*
 * PRECISION_OPT_TYPE options
 * LOW_PRECISION - Image and kernels are stored as 8-bit and 16-bit values respectively, and are typecasted dynamically during execution.
 * MID_PRECISION - Image and kernels are stored as 8-bit and 16-bit values respectively, but undergo a one-off typecast before compute, using scratchpad space to hold typecasted values. Extra scratchpad buffer space is needed to hold on to these temporary values.
 * HIGH_PRECISION - Everything is stored in memory as float. 
 */


#define L1_MAPS 5
#define L2_MAPS 50
#define L1_MAPS_PER_ECORE 1
#define L2_MAPS_PER_ECORE 4
#define L1_KERNEL_WIDTH 5
#define L2_KERNEL_WIDTH 5
#define L1_KERNEL_SIZE (5*5)
#define L2_KERNEL_SIZE (5*5)
#define L1_MAP_SIZE (11*11) //downsample by 2
#define L2_MAP_SIZE (3*3) //downsample by 2
#define IMAGE_SIZE (28*28)
#define IMAGE_WIDTH 28
#define L1_MAP_UNROLL_FACTOR 1
#define L2_MAP_UNROLL_FACTOR 1
#define DOWN_FAC1 2
#define DOWN_FAC2 2

#define BASE_ADDR 0x4000 //base address to start loading data at - might have to explore manually

//define types for data structures
#ifdef LOW_PRECISION
	#define GLOBAL_CONSTANTS_T unsigned
	#define L1_KERNEL_T uint8_t
	#define L2_KERNEL_T uint8_t
	#define L1_MAP_T uint16_t
	#define L2_MAP_T float //L2 map must be float -- for classification 
	#define IMAGE_T uint8_t
	#define MEM_ADDR_T unsigned //memory address type
	#define SCALE_T float
	#define INTERMEDIATE_T float //data type for storing intermediate values when operating each map
#endif

#ifdef MID_PRECISION
	#define GLOBAL_CONSTANTS_T unsigned
	#define L1_KERNEL_T uint8_t
	#define L2_KERNEL_T uint8_t
	#define L1_MAP_T uint16_t
	#define L2_MAP_T float //L2 map must be float -- for classification 
	#define IMAGE_T uint8_t
	#define MEM_ADDR_T unsigned //memory address type
	#define SCALE_T float
	#define INTERMEDIATE_T float //data type for storing intermediate values when operating each map
#endif

#ifdef HIGH_PRECISION
	#define GLOBAL_CONSTANTS_T unsigned
	#define L1_KERNEL_T float 
	#define L2_KERNEL_T float 
	#define L1_MAP_T float 
	#define L2_MAP_T float //L2 map must be float -- for classification 
	#define IMAGE_T float
	#define MEM_ADDR_T unsigned //memory address type
	#define SCALE_T float
	#define INTERMEDIATE_T float //data type for storing intermediate values when operating each map
#endif

unsigned FREE_SPACE = 0x7800 - BASE_ADDR; //let's keep stack size fixed at 2kB
unsigned curr_address = BASE_ADDR;

unsigned e_malloc(unsigned size, unsigned align);

int main (int argc, char **argv) {
	
	//3 global constants per eCore -- timesteps, L1_num_maps & L2_num_maps
	unsigned globals = e_malloc(3*sizeof(GLOBAL_CONSTANTS_T),sizeof(GLOBAL_CONSTANTS_T)); 
	unsigned image = e_malloc(IMAGE_SIZE*sizeof(IMAGE_T),sizeof(IMAGE_T));
	unsigned L1_kernel = e_malloc(L1_KERNEL_SIZE*L1_MAPS_PER_ECORE*sizeof(L1_KERNEL_T),sizeof(L1_KERNEL_T));
	unsigned L1_kernel_scale = e_malloc(L1_MAPS_PER_ECORE*sizeof(SCALE_T),sizeof(SCALE_T)); 
	unsigned L2_kernel = e_malloc(L2_KERNEL_SIZE*L2_MAPS_PER_ECORE*sizeof(L2_KERNEL_T),sizeof(L2_KERNEL_T));
	unsigned L2_kernel_scale = e_malloc(L2_MAPS_PER_ECORE*sizeof(SCALE_T),sizeof(SCALE_T)); 
	unsigned L1_maps = e_malloc(L1_MAP_SIZE*L1_MAPS_PER_ECORE*sizeof(L1_MAP_T),sizeof(L1_MAP_T));
	unsigned L2_maps = e_malloc(L2_MAP_SIZE*L2_MAPS_PER_ECORE*sizeof(L2_MAP_T),sizeof(L2_MAP_T));
	unsigned l1map_addr = e_malloc(L2_MAPS_PER_ECORE*L1_MAPS*sizeof(MEM_ADDR_T),sizeof(MEM_ADDR_T));
	unsigned l1map_state = e_malloc(L1_MAPS*L1_MAP_SIZE*sizeof(L1_MAP_T),sizeof(L1_MAP_T));
	unsigned mailbox = e_malloc(4*sizeof(unsigned),sizeof(unsigned));

	//let's work out scratchpad usage
	//inside L1
	unsigned scratch_s = 0;
	unsigned conv_size = IMAGE_WIDTH - L1_KERNEL_WIDTH +1;
	unsigned filter2D_out_size = conv_size*conv_size*sizeof(INTERMEDIATE_T);
	scratch_s += L1_MAP_UNROLL_FACTOR*filter2D_out_size;
#ifdef MID_PRECISION
	unsigned l1_kernel_floats = L1_MAP_UNROLL_FACTOR*L1_KERNEL_SIZE*sizeof(float);
	unsigned l1_image_floats = IMAGE_SIZE*sizeof(float);
	scratch_s += l1_image_floats + l1_kernel_floats;
#endif
//	printf("L1 scratchpad size = %d\n",scratch_s);

	//inside L2
	unsigned temp = 0;
	conv_size = conv_size/DOWN_FAC1 - L2_KERNEL_WIDTH + 1; //conv_size has decreased
	temp += L2_MAP_UNROLL_FACTOR*conv_size*conv_size*sizeof(INTERMEDIATE_T);//number of filter2D_out buffers
	temp += L1_MAP_SIZE*sizeof(L1_MAP_T);//accum vector
#ifdef MID_PRECISION
	unsigned l2_kernel_floats = L2_MAP_UNROLL_FACTOR*L2_KERNEL_SIZE*sizeof(float);
	unsigned l1_map_floats = L1_MAP_SIZE*sizeof(float);
	temp += l2_kernel_floats + l1_map_floats;
#endif
//	printf("L2 scratchpad size = %d\n",temp);
	
	if (temp > scratch_s)
		scratch_s = temp;

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
	printf("#define L1_KERNEL_SCALE_ADDR 0x%x\n",L1_kernel_scale);
	printf("#define L2_KERNEL_SCALE_ADDR 0x%x\n",L2_kernel_scale);
	printf("#define L1_MAPS_ADDR 0x%x\n",L1_maps);
	printf("#define L2_MAPS_ADDR 0x%x\n",L2_maps);
	printf("#define L2_L1_MAP_ADDR 0x%x\n",l1map_addr);
	printf("#define L2_L1_MAP_STATE_ADDR 0x%x\n",l1map_state);
	printf("#define DONE_ADDR 0x%x\n",mailbox);
	printf("#define SCRATCHPAD_ADDR 0x%x\n",scratchpad);

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

