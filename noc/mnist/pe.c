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
GLOBAL_CONSTANTS_T *timesteps, *L1_num_maps, *L2_num_maps;
MEM_ADDR_T *l1map_addr;
IMAGE_T *image;
KERNEL_T *L1_kernel, *L2_kernel;
MAP_T *L1_maps, *L2_maps, *l1map_state;
INTERMEDIATE_T *scratchpad;
SCALE_T *L1_kernel_scale, *L2_kernel_scale; 

//function declarations
void filter2D(KERNEL_T *kernel, MAP_T *src, INTERMEDIATE_T *dest, unsigned kernel_width, unsigned src_width, SCALE_T scale_fix);
void avepool_subsample(INTERMEDIATE_T *src, MAP_T *dest, unsigned stride, unsigned src_width);

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
	L1_num_maps = (GLOBAL_CONSTANTS_T *)(timesteps + 1);
	L2_num_maps = (GLOBAL_CONSTANTS_T *)(L1_num_maps + 1);
	image = (IMAGE_T *)IMAGE_ADDR;
	L1_kernel = (KERNEL_T *)L1_KERNEL_ADDR;
	L1_kernel_scale = (SCALE_T *)L1_KERNEL_SCALE_ADDR;
	L2_kernel = (KERNEL_T *)L2_KERNEL_ADDR;
	L2_kernel_scale = (SCALE_T *)L2_KERNEL_SCALE_ADDR;
	L1_maps = (MAP_T *)L1_MAPS_ADDR;
	L2_maps = (MAP_T *)L2_MAPS_ADDR;
	l1map_addr = (MEM_ADDR_T *)L2_L1_MAP_ADDR;
	l1map_state = (MAP_T *)L2_L1_MAP_STATE_ADDR;
	
	scratchpad = (INTERMEDIATE_T *)SCRATCHPAD_ADDR;

	int i,j,k,t;

	for (t=0;t<*timesteps;t++){
		//setup and initialize scratchpad memory for L1 compute
		unsigned conv_size = 22;

		//pointers to different temporary structures
		INTERMEDIATE_T *filter2D_out = (INTERMEDIATE_T *)(scratchpad);

		// ----------- layer 1 ------------ //
		for (i=0;i<*L1_num_maps;i++){
			KERNEL_T *kernel = (KERNEL_T *)(L1_kernel + (i*L1_KERNEL_SIZE));
			MAP_T *map = (MAP_T *)(L1_maps + (i*L1_MAP_SIZE));
			SCALE_T scale = L1_kernel_scale[i];

			filter2D(kernel,image,filter2D_out,L1_KERNEL_WIDTH,IMAGE_WIDTH,scale);
			avepool_subsample(filter2D_out,map,DOWN_FAC1,conv_size);
		}

		//barrier so that L1 computes are done before L2 starts
		e_barrier(barriers, tgt_bars);

		// ----------- layer 2 ------------ //
		e_ctimer_set(E_CTIMER_0, E_CTIMER_MAX);
		e_ctimer_start(E_CTIMER_0, E_CTIMER_CLK);
		__asm__ __volatile__("ldr %0, [%1]":"=r"(time_s):"r"(ctimer):);
		
		conv_size = 7;
		INTERMEDIATE_T *accum = (INTERMEDIATE_T *)(filter2D_out + conv_size*conv_size);

		//acccum + convolve once
		//fully-connected, so fetch 5 maps, and accum them, before convolve
		MEM_ADDR_T *src = (MEM_ADDR_T *)(l1map_addr[0]);
		MEM_ADDR_T *src1 = (MEM_ADDR_T *)(l1map_addr[1]);
		MEM_ADDR_T *src2 = (MEM_ADDR_T *)(l1map_addr[2]);
		MEM_ADDR_T *src3 = (MEM_ADDR_T *)(l1map_addr[3]);
		MEM_ADDR_T *src4 = (MEM_ADDR_T *)(l1map_addr[4]);
		MAP_T *dest = (MAP_T *)(l1map_state); 
		MAP_T *dest1 = (MAP_T *)(l1map_state + L1_MAP_SIZE);
		MAP_T *dest2 = (MAP_T *)(l1map_state + 2*L1_MAP_SIZE);
		MAP_T *dest3 = (MAP_T *)(l1map_state + 3*L1_MAP_SIZE);
		MAP_T *dest4 = (MAP_T *)(l1map_state + 4*L1_MAP_SIZE);

		e_dma_copy(dest,src,L1_MAP_SIZE*sizeof(MAP_T));
		e_dma_copy(dest1,src1,L1_MAP_SIZE*sizeof(MAP_T));
		e_dma_copy(dest2,src2,L1_MAP_SIZE*sizeof(MAP_T));
		e_dma_copy(dest3,src3,L1_MAP_SIZE*sizeof(MAP_T));
		e_dma_copy(dest4,src4,L1_MAP_SIZE*sizeof(MAP_T));

		//accum
		int cs_cs = 49;//conv_size*conv_size
		for (i=0;i<cs_cs;i++){
			accum[i] = 0;//init to zero
			//accum[i] = dest[i] + dest1[i] + dest2[i] + dest3[i] + dest4[i];
		}

		for (i=0;i<*L2_num_maps;i++){
			
			KERNEL_T *l2_kernel = (KERNEL_T *)(L2_kernel + (i*L2_KERNEL_SIZE));
			SCALE_T scalel2 = L2_kernel_scale[i];
			MAP_T *l2_map = (MAP_T *)(L2_maps + (i*L2_MAP_SIZE));
		
			//convolve-add
			filter2D(l2_kernel,dest,filter2D_out,L2_KERNEL_WIDTH,L1_MAP_WIDTH,scalel2);
			for (j=0;j<cs_cs;j++)
				accum[j] += filter2D_out[j];
			filter2D(l2_kernel,dest1,filter2D_out,L2_KERNEL_WIDTH,L1_MAP_WIDTH,scalel2);
			for (j=0;j<cs_cs;j++)
				accum[j] += filter2D_out[j];
			filter2D(l2_kernel,dest2,filter2D_out,L2_KERNEL_WIDTH,L1_MAP_WIDTH,scalel2);
			for (j=0;j<cs_cs;j++)
				accum[j] += filter2D_out[j];
			filter2D(l2_kernel,dest3,filter2D_out,L2_KERNEL_WIDTH,L1_MAP_WIDTH,scalel2);
			for (j=0;j<cs_cs;j++)
				accum[j] += filter2D_out[j];
			filter2D(l2_kernel,dest4,filter2D_out,L2_KERNEL_WIDTH,L1_MAP_WIDTH,scalel2);
			for (j=0;j<cs_cs;j++)
				accum[j] += filter2D_out[j];

			//reduce to 6*6
			conv_size = 6;
			int old_conv_size = 7;
			for (j=0;j<conv_size;j++){
				for (k=0;k<conv_size;k++){
					filter2D_out[j*conv_size+k] = accum[j*old_conv_size+k];
				}
			}

			//maxpool + subsample
			avepool_subsample(filter2D_out,l2_map,DOWN_FAC2,conv_size);
		}
		
		__asm__ __volatile__("ldr %0, [%1]":"=r"(time_e):"r"(ctimer):);
		e_ctimer_stop(E_CTIMER_0);
		mailboxes[1] += time_s - time_e - timer_offset;
	}

	*mailboxes = 0xdeadbeef;

	return 0;
}

void filter2D(KERNEL_T *kernel, MAP_T *src, INTERMEDIATE_T *dest, unsigned kernel_width, unsigned src_width, SCALE_T scale_fix){
	unsigned row,col,kernel_row,kernel_col,i;
	int cntr[kernel_width];
	unsigned o_cntr = 0;
	unsigned conv_size = src_width - kernel_width + 1;
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

			sop = sop*scale_fix;
			*(dest + o_cntr) = sop > 0 ? sop : 0;//non-linearity

			o_cntr++;
			for (i=0;i<kernel_width;i++)
				cntr[i]++;
		}
	}
}

void avepool_subsample(INTERMEDIATE_T *src, MAP_T *dest, unsigned stride, unsigned src_width){
	unsigned row,col,i,j;
	int cntr[stride];
	unsigned o_cntr = 0;

	for (row=0;row<src_width;row+=stride){
		for (i=0;i<stride;i++)
			cntr[i] = (row+i)*src_width;
		for (col=0;col<src_width;col+=stride){

			//unrolled version - 2x2
			INTERMEDIATE_T sum = src[cntr[0]]+src[cntr[0]+1]+src[cntr[1]]+src[cntr[1]+1];
			sum = sum*0.25f;//divide by 4

			dest[o_cntr] = sum;
			o_cntr++;

			for (i=0;i<stride;i++)
				cntr[i] += stride;
		}
	}
}
