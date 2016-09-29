#include <stdint.h>
#include "e_lib.h"

#include "types.h"
#include "parameters.h"
#include "address.h"
#include "macros.h"

// global memory management -- stack start address 
asm(".global __stack_start__;");
asm(".set __stack_start__,0x7800;");
asm(".global __heap_start__;");
asm(".set __heap_start__,0x0000;");

GLOBAL_CONSTANTS_T *num_maps;
IMAGE_T *image;
KERNEL_T *kernels;
MAP_T *maps;
INTERMEDIATE_T *scratchpad;
SCALE_T *kernel_scales;
unsigned *mailboxes;

//function declarations
void filter2D(KERNEL_T *kernel, IMAGE_T *src, INTERMEDIATE_T *dest, unsigned kernel_width, unsigned src_width, unsigned conv_size, SCALE_T scale_fix);
void maxpool_subsample(INTERMEDIATE_T *src, MAP_T *dest, unsigned stride, unsigned src_width);

int main(void){

	mailboxes = (GLOBAL_CONSTANTS_T *)DONE_ADDR; //4 mailbox locations available per core
	*mailboxes = 0xbeefdead;

	/*allocate memory based on number of nodes and edges*/
	num_maps = (GLOBAL_CONSTANTS_T *)GLOBAL_CONSTANTS_ADDR;
	image = (IMAGE_T *)IMAGE_ADDR;
	kernels = (KERNEL_T *)KERNEL_ADDR;
	kernel_scales = (SCALE_T *)KERNEL_SCALE_ADDR;
	maps = (MAP_T *)MAPS_ADDR;
	scratchpad = (INTERMEDIATE_T *)SCRATCHPAD_ADDR;
	//pointers to different temporary structures
	unsigned conv_size = IMAGE_WIDTH - KERNEL_WIDTH + 1;
	PTR_UNROLL(MAP_UNROLL_FACTOR,BUFFER_PTR,0);

	int i;
	for (i=0;i<*num_maps;i+=MAP_UNROLL_FACTOR){
		PTR_UNROLL(MAP_UNROLL_FACTOR,KERNEL_PTR,i);
		PTR_UNROLL(MAP_UNROLL_FACTOR,MAP_PTR,i);
		PTR_UNROLL(MAP_UNROLL_FACTOR,SCALE_PTR,i);

		FUNC_UNROLL(MAP_UNROLL_FACTOR,FILTER2D);
		FUNC_UNROLL(MAP_UNROLL_FACTOR,MAXPOOL_SUBSAMPLE);
	}
	
	*mailboxes = 0xdeadbeef;

	return 0;
}

void filter2D(KERNEL_T *kernel, IMAGE_T *src, INTERMEDIATE_T *dest, unsigned kernel_width, unsigned src_width, unsigned conv_size, SCALE_T scale_fix){
	unsigned row,col,kernel_row,kernel_col,i;
	int cntr[kernel_width];
	unsigned o_cntr = 0;
	for (row=0;row<conv_size;row++){
		for (i=0;i<kernel_width;i++)
			cntr[i] = (row+i)*src_width;
		for (col=0;col<conv_size;col++){
			INTERMEDIATE_T sop = 0.0f;
			
			sop += (kernel[0])*(src[cntr[0]]);
			sop += (kernel[1])*(src[cntr[0]+1]);
			sop += (kernel[2])*(src[cntr[0]+2]);
			sop += (kernel[3])*(src[cntr[0]+3]);
			sop += (kernel[4])*(src[cntr[0]+4]);

			sop += (kernel[5])*(src[cntr[1]]);
			sop += (kernel[6])*(src[cntr[1]+1]);
			sop += (kernel[7])*(src[cntr[1]+2]);
			sop += (kernel[8])*(src[cntr[1]+3]);
			sop += (kernel[9])*(src[cntr[1]+4]);

			sop += (kernel[10])*(src[cntr[2]]);
			sop += (kernel[11])*(src[cntr[2]+1]);
			sop += (kernel[12])*(src[cntr[2]+2]);
			sop += (kernel[13])*(src[cntr[2]+3]);
			sop += (kernel[14])*(src[cntr[2]+4]);

			sop += (kernel[15])*(src[cntr[3]]);
			sop += (kernel[16])*(src[cntr[3]+1]);
			sop += (kernel[17])*(src[cntr[3]+2]);
			sop += (kernel[18])*(src[cntr[3]+3]);
			sop += (kernel[19])*(src[cntr[3]+4]);

			sop += (kernel[20])*(src[cntr[4]]);
			sop += (kernel[21])*(src[cntr[4]+1]);
			sop += (kernel[22])*(src[cntr[4]+2]);
			sop += (kernel[23])*(src[cntr[4]+3]);
			sop += (kernel[24])*(src[cntr[4]+4]);

			*(dest + o_cntr) = sop*scale_fix;
			o_cntr++;
			for (i=0;i<kernel_width;i++)
				cntr[i]++;
		}
	}
}

void maxpool_subsample(INTERMEDIATE_T *src, MAP_T *dest, unsigned stride, unsigned src_width){
	unsigned row,col,i,j;
	int cntr[stride];
	unsigned o_cntr = 0;

	for (row=0;row<src_width;row+=stride){
		for (i=0;i<stride;i++)
			cntr[i] = (row+i)*src_width;
		for (col=0;col<src_width;col+=stride){

			//unrolled version - 2x2
			float sum = src[cntr[0]]+src[cntr[0]+1]+src[cntr[1]]+src[cntr[1]+1];
			sum = sum*0.25f;//divide by 4

			dest[o_cntr] = sum;
			o_cntr++;

			for (i=0;i<stride;i++)
				cntr[i] += stride;
		}
	}
}
