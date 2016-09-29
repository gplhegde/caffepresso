/* 
 * Deeplearning MNIST stack on the parallella.
 *
 * December 2015 -- Sid
 */

#include <stdint.h>
#include "e_lib.h"

#include "types.h"
#include "address.h"
#include "parameters.h"

volatile e_barrier_t  barriers[NUM_PES];
         e_barrier_t *tgt_bars[NUM_PES];

// global memory management -- stack start address 
asm(".global __stack_start__;");
asm(".set __stack_start__,0x7800;");
asm(".global __heap_start__;");
asm(".set __heap_start__,0x0000;");

FLAG_T *mailboxes;
GLOBAL_CONSTANTS_T *timesteps;
IMAGE_T *image;
KERNEL_T *L1_kernel, *L2_kernel, *L3_kernel;
MAP_T *maps;
INTERMEDIATE_T *scratchpad;
SCALE_T *L1_kernel_scale, *L2_kernel_scale, *L3_kernel_scale; 

//function declarations
void filter2D(KERNEL_T *kernel, MAP_T *src, INTERMEDIATE_T *dest, unsigned kernel_width, unsigned src_width, SCALE_T scale);
void pool_subsample(INTERMEDIATE_T *src, MAP_T *dest, unsigned window, unsigned stride, unsigned src_width, int isMaxPool);

#define CTIMERADD (0x000f0438)

unsigned timer_offset;

int main(void){

	unsigned time_s, time_e;
	unsigned ctimer, row, col;

	//setup timer
	row = e_group_config.core_row;
	col = e_group_config.core_col;
	ctimer = (unsigned)e_get_global_address(row, col, (void *)CTIMERADD);

	//record the offset
	e_ctimer_set(E_CTIMER_0, E_CTIMER_MAX);
	e_ctimer_start(E_CTIMER_0, E_CTIMER_CLK);
	__asm__ __volatile__("ldr %0, [%1]":"=r"(time_s):"r"(ctimer):);
	__asm__ __volatile__("ldr %0, [%1]":"=r"(time_e):"r"(ctimer):);
	
	mailboxes = (FLAG_T *)DONE_ADDR; //4 mailbox locations available per core
	*mailboxes = 0xbeefdead;
	mailboxes[1] = 0;
	
	//initialize barriers
	e_barrier_init(barriers, tgt_bars);

	/*allocate memory based on number of nodes and edges*/
	timesteps = (GLOBAL_CONSTANTS_T *)GLOBAL_CONSTANTS_ADDR;
	image = (IMAGE_T *)IMAGE_ADDR;
	L1_kernel = (KERNEL_T *)L1_KERNEL_ADDR;
	L1_kernel_scale = (SCALE_T *)L1_KERNEL_SCALE_ADDR;
	L2_kernel = (KERNEL_T *)L2_KERNEL_ADDR;
	L2_kernel_scale = (SCALE_T *)L2_KERNEL_SCALE_ADDR;
	L3_kernel = (KERNEL_T *)L3_KERNEL_ADDR;
	L3_kernel_scale = (SCALE_T *)L3_KERNEL_SCALE_ADDR;
	maps = (MAP_T *)MAPS_ADDR; //only need to store last computed maps..
	scratchpad = (INTERMEDIATE_T *)(maps + L1_MAX_MAPS_PER_ECORE*L1_MAP_SIZE);//dynamically allocate ptr at each layer

	//global addresses pre-computed for local accum arrays stored in each eCore
	MEM_ADDR_T src_addresses[NUM_PES] = MAPS_ECORE_ADDRS;

	int i,j,k,t;

	for (t=0;t<*timesteps;t++){
		//--------------- layer 1 ----------------/
		//setup and initialize scratchpad memory for L1 compute
		INTERMEDIATE_T *filter2D_out0 = (INTERMEDIATE_T *)(scratchpad);

		//unroll factor = 2, i.e. evaluate 2 maps at a time..
		unsigned conv_size = 32;
		KERNEL_T *kernel0 = (KERNEL_T *)(L1_kernel);
		KERNEL_T *kernel1 = (KERNEL_T *)(L1_kernel + L1_KERNEL_SIZE);
		MAP_T *map0 = (MAP_T *)(maps);
		MAP_T *map1 = (MAP_T *)(maps + L1_MAP_SIZE);

		filter2D(kernel0,image,filter2D_out0,L1_KERNEL_WIDTH,IMAGE_WIDTH,L1_kernel_scale[0]);
		pool_subsample(filter2D_out0,map0,WINDOW_SIZE1,DOWN_FAC1,conv_size,1);
		filter2D(kernel1,image,filter2D_out0,L1_KERNEL_WIDTH,IMAGE_WIDTH,L1_kernel_scale[1]);
		pool_subsample(filter2D_out0,map1,WINDOW_SIZE1,DOWN_FAC1,conv_size,1);

		//barrier so that L1 computes are done before L2 starts
		e_barrier(barriers, tgt_bars);
	
		// ----------- layer 2 ------------ /
		unsigned pad_width = 20;
		unsigned pad_size = 400;
		conv_size = 16;
		MAP_T *dest0 = (MAP_T *)(scratchpad);
		MAP_T *dest_pad = (MAP_T *)(dest0 + L1_MAP_SIZE*L1_MAX_MAPS_PER_ECORE);
		filter2D_out0 = (INTERMEDIATE_T *)(dest_pad + pad_size*L1_MAX_MAPS_PER_ECORE);
		INTERMEDIATE_T *filter2D_out1 = (INTERMEDIATE_T *)(filter2D_out0 + L1_MAP_SIZE);
		MAP_T *accum0 = (MAP_T *)(image); //can use image space..
		MAP_T *accum1 = (MAP_T *)(accum0 + L1_MAP_SIZE); //can use image space..

		//initialize accum to zeros
		for (i=0;i<L1_MAP_SIZE;i++){
			accum0[i] = 0.0f;
			accum1[i] = 0.0f;
		}
	
		//2 L2 maps per eCore
		//fully-connected, so fetch 16 local-accums of maps, and accum them, before convolve
		MEM_ADDR_T *src0 = (MEM_ADDR_T *)(src_addresses[0]);
		e_dma_copy(dest0,src0,L1_MAP_SIZE*L1_MAX_MAPS_PER_ECORE*sizeof(MAP_T));
		for (j=0;j<NUM_PES;j++){
			src0 = (MEM_ADDR_T *)(src_addresses[j+1]);
			//pad by 2...
			for (k=0;k<pad_size*L1_MAX_MAPS_PER_ECORE;k++){
				dest_pad[k] = 0.0f; //init to zero first
			}
			for (k=2;k<pad_width-2;k++){
				for (i=2;i<pad_width-2;i++){
					dest_pad[k*pad_width+i] = dest0[(k-2)*L1_MAP_WIDTH+(i-2)];
					dest_pad[pad_size+k*pad_width+i]=dest0[L1_MAP_SIZE+(k-2)*L1_MAP_WIDTH+(i-2)];
				}
			}

			mailboxes[1] = (j+1);
		
			if (j != NUM_PES-1)
				e_dma_copy(dest0,src0,L1_MAP_SIZE*L1_MAX_MAPS_PER_ECORE*sizeof(MAP_T));//transfer next..
			//convolve
			KERNEL_T *kernel0 = (KERNEL_T *)(L2_kernel);
			filter2D(kernel0,dest_pad,filter2D_out0,L2_KERNEL_WIDTH,pad_width,L2_kernel_scale[0]);
			filter2D(kernel0,(dest_pad+pad_size),filter2D_out1,L2_KERNEL_WIDTH,pad_width,L2_kernel_scale[0]);
			for (k=0;k<L1_MAP_SIZE*L1_MAX_MAPS_PER_ECORE;k++){
				accum0[k] += filter2D_out0[k] + filter2D_out1[k];
			}
			KERNEL_T *kernel1 = (KERNEL_T *)(L2_kernel + L2_KERNEL_SIZE);
			filter2D(kernel1,dest_pad,filter2D_out0,L2_KERNEL_WIDTH,pad_width,L2_kernel_scale[1]);
			filter2D(kernel1,(dest_pad+pad_size),filter2D_out1,L2_KERNEL_WIDTH,pad_width,L2_kernel_scale[1]);
			for (k=0;k<L1_MAP_SIZE*L1_MAX_MAPS_PER_ECORE;k++){
				accum1[k] += filter2D_out0[k] + filter2D_out1[k];
			}
		}
		
		//barrier so that all transfers and accums are complete before doing L2 pooling..  
		e_barrier(barriers, tgt_bars);

		pool_subsample(accum0,maps,WINDOW_SIZE2,DOWN_FAC2,conv_size,0); //ave-pool
		pool_subsample(accum1,(maps+L2_MAP_SIZE),WINDOW_SIZE2,DOWN_FAC2,conv_size,0); //ave-pool

		//barrier so that all L2 is complete before starting L3.. 
		e_barrier(barriers, tgt_bars);
		// ----------- layer 3 ------------ /
		pad_width = 12;
		pad_size = 144;
		conv_size = 8;
		dest0 = (MAP_T *)(scratchpad);
		dest_pad = (MAP_T *)(dest0 + L2_MAP_SIZE*L2_MAX_MAPS_PER_ECORE);
		filter2D_out0 = (INTERMEDIATE_T *)(dest_pad + pad_size*L2_MAX_MAPS_PER_ECORE);
		filter2D_out1 = (INTERMEDIATE_T *)(filter2D_out0 + L2_MAP_SIZE);
		accum0 = (MAP_T *)(image); //can use image space..
		accum1 = (MAP_T *)(accum0 + L2_MAP_SIZE); //can use image space..
		MAP_T *accum2 = (MAP_T *)(accum1 + L2_MAP_SIZE); //can use image space..
		MAP_T *accum3 = (MAP_T *)(accum2 + L2_MAP_SIZE); //can use image space..

		//initialize accum to zeros
		for (i=0;i<L2_MAP_SIZE;i++){
			accum0[i] = 0.0f;
			accum1[i] = 0.0f;
			accum2[i] = 0.0f;
			accum3[i] = 0.0f;
		}
	
		//4 L3 maps per eCore
		//fully-connected, so fetch 16 local-accums of maps, and accum them, before convolve
		src0 = (MEM_ADDR_T *)(src_addresses[0]);
		e_dma_copy(dest0,src0,L2_MAP_SIZE*L2_MAX_MAPS_PER_ECORE*sizeof(MAP_T));
		for (j=0;j<NUM_PES;j++){
			src0 = (MEM_ADDR_T *)(src_addresses[j+1]);
			//pad by 2...
			for (k=0;k<pad_size*L2_MAX_MAPS_PER_ECORE;k++){
				dest_pad[k] = 0.0f; //init to zero first
			}
			for (k=2;k<pad_width-2;k++){
				for (i=2;i<pad_width-2;i++){
					dest_pad[k*pad_width+i] = dest0[(k-2)*L2_MAP_WIDTH+(i-2)];
					dest_pad[pad_size+k*pad_width+i]=dest0[L2_MAP_SIZE+(k-2)*L2_MAP_WIDTH+(i-2)];
				}
			}
			if (j != NUM_PES-1)
				e_dma_copy(dest0,src0,L2_MAP_SIZE*L2_MAX_MAPS_PER_ECORE*sizeof(MAP_T));//transfer next..
			//convolve
			KERNEL_T *kernel0 = (KERNEL_T *)(L3_kernel);
			filter2D(kernel0,dest_pad,filter2D_out0,L3_KERNEL_WIDTH,pad_width,L3_kernel_scale[0]);
			filter2D(kernel0,(dest_pad+pad_size),filter2D_out1,L3_KERNEL_WIDTH,pad_width,L3_kernel_scale[0]);
			for (k=0;k<L2_MAP_SIZE*L2_MAX_MAPS_PER_ECORE;k++){
				accum0[k] += filter2D_out0[k] + filter2D_out1[k];
			}
			KERNEL_T *kernel1 = (KERNEL_T *)(L3_kernel + L3_KERNEL_SIZE);
			filter2D(kernel1,dest_pad,filter2D_out0,L3_KERNEL_WIDTH,pad_width,L3_kernel_scale[1]);
			filter2D(kernel1,(dest_pad+pad_size),filter2D_out1,L3_KERNEL_WIDTH,pad_width,L3_kernel_scale[1]);
			for (k=0;k<L2_MAP_SIZE*L2_MAX_MAPS_PER_ECORE;k++){
				accum1[k] += filter2D_out0[k] + filter2D_out1[k];
			}
			KERNEL_T *kernel2 = (KERNEL_T *)(L3_kernel + 2*L3_KERNEL_SIZE);
			filter2D(kernel2,dest_pad,filter2D_out0,L3_KERNEL_WIDTH,pad_width,L3_kernel_scale[2]);
			filter2D(kernel2,(dest_pad+pad_size),filter2D_out1,L3_KERNEL_WIDTH,pad_width,L3_kernel_scale[2]);
			for (k=0;k<L2_MAP_SIZE*L2_MAX_MAPS_PER_ECORE;k++){
				accum2[k] += filter2D_out0[k] + filter2D_out1[k];
			}
			KERNEL_T *kernel3 = (KERNEL_T *)(L3_kernel + 3*L3_KERNEL_SIZE);
			filter2D(kernel3,dest_pad,filter2D_out0,L3_KERNEL_WIDTH,pad_width,L3_kernel_scale[3]);
			filter2D(kernel3,(dest_pad+pad_size),filter2D_out1,L3_KERNEL_WIDTH,pad_width,L3_kernel_scale[3]);
			for (k=0;k<L2_MAP_SIZE*L2_MAX_MAPS_PER_ECORE;k++){
				accum3[k] += filter2D_out0[k] + filter2D_out1[k];
			}
		}
		
		//barrier so that all transfers and accums are complete before doing L3 pooling..  
		e_barrier(barriers, tgt_bars);

		pool_subsample(accum0,maps,WINDOW_SIZE3,DOWN_FAC3,conv_size,0); //ave-pool
		pool_subsample(accum1,(maps+L3_MAP_SIZE),WINDOW_SIZE3,DOWN_FAC3,conv_size,0); //ave-pool
		pool_subsample(accum2,(maps+2*L3_MAP_SIZE),WINDOW_SIZE3,DOWN_FAC3,conv_size,0); //ave-pool
		pool_subsample(accum3,(maps+3*L3_MAP_SIZE),WINDOW_SIZE3,DOWN_FAC3,conv_size,0); //ave-pool
	}	
		
	*mailboxes = 0xdeadbeef;

	return 0;
}

void filter2D(KERNEL_T *kernel, MAP_T *src, INTERMEDIATE_T *dest, 
	unsigned kernel_width, unsigned src_width, SCALE_T scale){

	unsigned row,col,kernel_row,kernel_col,i,j;
	int cntr[kernel_width];
	unsigned o_cntr = 0;
	unsigned conv_size = src_width - kernel_width + 1;
	for (row=0;row<conv_size;row++){
		for (i=0;i<kernel_width;i++){
			cntr[i] = (row+i)*src_width;
		}
		for (col=0;col<conv_size;col++){
			INTERMEDIATE_T sop = 0.0f;
			
			sop += kernel[0]*src[cntr[0]];
			sop += kernel[1]*src[cntr[0]+1];
			sop += kernel[2]*src[cntr[0]+2];
			sop += kernel[3]*src[cntr[0]+3];
			sop += kernel[4]*src[cntr[0]+4];

			sop += kernel[5]*src[cntr[1]];
			sop += kernel[6]*src[cntr[1]+1];
			sop += kernel[7]*src[cntr[1]+2];
			sop += kernel[8]*src[cntr[1]+3];
			sop += kernel[9]*src[cntr[1]+4];

			sop += kernel[10]*src[cntr[2]];
			sop += kernel[11]*src[cntr[2]+1];
			sop += kernel[12]*src[cntr[2]+2];
			sop += kernel[13]*src[cntr[2]+3];
			sop += kernel[14]*src[cntr[2]+4];

			sop += kernel[15]*src[cntr[3]];
			sop += kernel[16]*src[cntr[3]+1];
			sop += kernel[16]*src[cntr[3]+2];
			sop += kernel[18]*src[cntr[3]+3];
			sop += kernel[19]*src[cntr[3]+4];

			sop += kernel[20]*src[cntr[4]];
			sop += kernel[21]*src[cntr[4]+1];
			sop += kernel[22]*src[cntr[4]+2];
			sop += kernel[23]*src[cntr[4]+3];
			sop += kernel[24]*src[cntr[4]+4];

			sop = sop*scale;
			*(dest + o_cntr) = sop;

			o_cntr++;
			for (i=0;i<kernel_width;i++)
				cntr[i]++;
		}
	}
}

void pool_subsample(INTERMEDIATE_T *src, MAP_T *dest, unsigned window, unsigned stride, unsigned src_width, int isMaxPool){
	unsigned row,col,i,j;
	int cntr[window];
	unsigned o_cntr = 0;

	for (row=0;row<src_width;row+=stride){
		for (i=0;i<stride;i++)
			cntr[i] = (row+i)*src_width;
		for (col=0;col<src_width;col+=stride){

			float sum = 0.0f;
			//unrolled version - 3x3 window size
			if (isMaxPool){ //maxpool + relu
				float c0,c1,c2,c3;
				c0 = src[cntr[0]] > src[cntr[0]+1] ? src[cntr[0]] : src[cntr[0]+1];
				c1 = src[cntr[0]+2] > src[cntr[1]] ? src[cntr[0]+2] : src[cntr[1]];
				c2 = src[cntr[1]+1] > src[cntr[1]+2] ? src[cntr[1]+1] : src[cntr[1]+2];
				c3 = src[cntr[2]] > src[cntr[2]+1] ? src[cntr[2]] : src[cntr[2]+1];
				sum = src[cntr[2]+2] > sum ? src[cntr[2]+2] : sum;
				c0 = c0 > c1 ? c0 : c1;
				c2 = c2 > c3 ? c2 : c3;
				sum = sum > c0 ? sum : c0;
				sum = sum > c2 ? sum : c2;
			} else {
				sum = src[cntr[0]]+src[cntr[0]+1]+src[cntr[0]+2]+src[cntr[1]]+src[cntr[1]+1]+src[cntr[1]+2]+src[cntr[2]]+src[cntr[2]+1]+src[cntr[2]+2];
				sum = sum*0.111111f;//divide by 9.. some preicison might be lost here...
				sum = sum > 0.0f ? sum : 0.0f;
			}
			

			dest[o_cntr] = sum;
			o_cntr++;

			for (i=0;i<stride;i++)
				cntr[i] += stride;
		}
	}
}
