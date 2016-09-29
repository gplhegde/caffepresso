#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/*
 * PRECISION_OPT_TYPE options
 * LOW_PRECISION - Image and kernels are stored as 8-bit and 16-bit values respectively, and are typecasted dynamically during execution.
 * MID_PRECISION - Image and kernels are stored as 8-bit and 16-bit values respectively, but undergo a one-off typecast before compute, using scratchpad space to hold typecasted values. Extra scratchpad buffer space is needed to hold on to these temporary values.
 * HIGH_PRECISION - Everything is stored in memory as float. 
 */

/*
#define KERNEL_WIDTH 7
#define KERNEL_SIZE (7*7)
#define MAP_SIZE (11*11)
#define IMG_SIZE (28*28)
#define IMG_WIDTH 28
#define MAP_UNROLL_FACTOR 1
*/

//#define BASE_ADDR 0x2000 //base address to start loading data at - might have to explore manually
#define GLOBAL_CONSTANTS_T unsigned

//define types for data structures
#ifdef LOW_PRECISION
	#define KERNEL_T uint8_t
	#define MAP_T uint16_t
	#define IMAGE_T uint8_t
	#define SCALE_T float
	#define INTERMEDIATE_T float //data type for storing intermediate values when operating each map
#endif

#ifdef MID_PRECISION
	#define KERNEL_T uint8_t
	#define MAP_T uint16_t
	#define IMAGE_T uint8_t
	#define SCALE_T float
	#define INTERMEDIATE_T float //data type for storing intermediate values when operating each map
#endif

#ifdef HIGH_PRECISION
	#define KERNEL_T float 
	#define MAP_T float 
	#define IMAGE_T float
	#define SCALE_T float
	#define INTERMEDIATE_T float //data type for storing intermediate values when operating each map
#endif

int FREE_SPACE = 0x7800;// - BASE_ADDR; //let's keep stack size fixed at 2kB
unsigned curr_address;

unsigned e_malloc(unsigned size, unsigned align);

int main (int argc, char **argv) {
	
	int IMG_WIDTH = atoi(argv[1]);
	int maps_per_ecore = atoi(argv[2]);
	int KERNEL_WIDTH = atoi(argv[3]);
	int DOWN_FAC = atoi(argv[4]);
	int MAP_UNROLL_FACTOR = atoi(argv[5]);
	int BASE_ADDR = strtol(argv[6],NULL,16);

	FREE_SPACE = FREE_SPACE - BASE_ADDR;
	curr_address = BASE_ADDR;

	//sanity check
	if (IMG_WIDTH < KERNEL_WIDTH){
		printf("Image cannot be smaller than kernel, you fool!\n");
		exit(1);
	}

	int MAP_WIDTH = IMG_WIDTH - KERNEL_WIDTH + 1;
	//sanity check
	if (MAP_WIDTH%DOWN_FAC != 0){
		printf("Unaligned downsampling is not currently supported..sorry.\n");
		exit(1);
	}

	MAP_WIDTH = MAP_WIDTH/DOWN_FAC;
	int MAP_SIZE = MAP_WIDTH*MAP_WIDTH;
	int KERNEL_SIZE = KERNEL_WIDTH*KERNEL_WIDTH;
	int IMG_SIZE = IMG_WIDTH*IMG_WIDTH;

	//sanity check
	if (maps_per_ecore%MAP_UNROLL_FACTOR != 0){
		printf("Unaligned map unroll evaluation not supported currenty..sorry.\n");
		exit(1);
	}

	//1 global constant per eCore -- num_maps
	unsigned globals = e_malloc(sizeof(GLOBAL_CONSTANTS_T),sizeof(GLOBAL_CONSTANTS_T)); 
	unsigned image = e_malloc(IMG_SIZE*sizeof(IMAGE_T),sizeof(IMAGE_T));
	unsigned kernel = e_malloc(KERNEL_SIZE*maps_per_ecore*sizeof(KERNEL_T),sizeof(KERNEL_T));
	unsigned kernel_scale = e_malloc(maps_per_ecore*sizeof(SCALE_T),sizeof(SCALE_T)); 
	unsigned maps = e_malloc(MAP_SIZE*maps_per_ecore*sizeof(MAP_T),sizeof(MAP_T));
	unsigned mailbox = e_malloc(4*sizeof(unsigned),sizeof(unsigned));

	//let's work out scratchpad usage
	//inside L1
	unsigned scratch_s = 0;
	unsigned conv_size = IMG_WIDTH - KERNEL_WIDTH + 1;
	unsigned filter2D_out_size = conv_size*conv_size*sizeof(INTERMEDIATE_T);
	scratch_s += MAP_UNROLL_FACTOR*filter2D_out_size;
#ifdef MID_PRECISION
	unsigned kernel_floats = MAP_UNROLL_FACTOR*KERNEL_SIZE*sizeof(float);
	unsigned image_floats = IMG_SIZE*sizeof(float);
	scratch_s += image_floats + kernel_floats;
#endif
	unsigned scratchpad = e_malloc(scratch_s,sizeof(INTERMEDIATE_T));

	/*
	printf("L1_kernel = 0x%x, L2_kernel = 0x%x\n",L1_kernel,L2_kernel);
	printf("L1_kernel_scale = 0x%x, L2_kernel_scale = 0x%x\n",L1_kernel_scale,L2_kernel_scale);
	printf("L1_maps = 0x%x, L2_maps = 0x%x\n",L1_maps,L2_maps);
	printf("l1map_addr = 0x%x, l1map_state = 0x%x\n",l1map_addr,l1map_state);
	printf("Mailboxes = 0x%x, scratchpad = 0x%x\n",mailbox,scratchpad);
	printf("SCRATCHPAD SIZE = %d bytes\n",scratch_s);
	printf("Available space = %d bytes\n",FREE_SPACE);
	printf("Stack space = 2kB\n");
	*/

	//success if you got this far..
	printf("\n");
	printf("SUCCESS\n");
	printf("\n");
	
	printf("FREE SPACE=%d\n\n",FREE_SPACE);
	printf("MAP WIDTH=%d\n\n",MAP_WIDTH);

	printf("#define GLOBAL_CONSTANTS_ADDR 0x%x\n",globals);
	printf("#define IMG_ADDR 0x%x\n",image);
	printf("#define KERNEL_ADDR 0x%x\n",kernel);
	printf("#define KERNEL_SCALE_ADDR 0x%x\n",kernel_scale);
	printf("#define MAPS_ADDR 0x%x\n",maps);
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

