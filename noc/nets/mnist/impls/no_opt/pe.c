/* 
 * Deeplearning stack on the parallella.
 *
 * September 2015 -- Sid
 *****************************************************************************************************
 *	0x2400 L1_num_maps -- unsigned 4 bytes (number of L1 maps on this core)
 *	0x2404 L2_num_maps -- unsigned 4 bytes (number of L2 maps on this core)
 *	0x2408 image -- uint8_t 1 byte per pixel (SIZE_IMAGE)
 *	0x2718 L1_kernel -- uint8_t 1 byte per cell (L1_KERNEL_SIZE*L1_MAX_MAPS)
 *	0x27dc L1_kernel_scale -- float 4 bytes per cell (L1_MAX_MAPS*4)
 *	0x27ec L2_kernel -- uint8_t 1 byte per cell (L2_KERNEL_SIZE*L2_MAX_MAPS)
 *	0x28b4 L2_kernel_scale -- float 4 bytes per cell (L2_MAX_MAPS*4)
 *	0x28d4 L1_maps -- uint16_t 2 byte per cell (L1_MAP_SIZE*L1_MAX_MAPS)
 *	0x2c9c L2_maps -- uint16_t 2 byte per cell (L2_MAP_SIZE*L2_MAX_MAPS) 
 *	0x2d9c l1map_addr -- unsigned 4 bytes per cell (L1_TO_L2*L2_MAX_MAPS*4)
 *	0x2edc l1map_state -- uint16_t 2 byte per cell (UNROLL_FACTOR*L1_MAP_SIZE) 
 *	0x30c0 l2kernel_addr -- unsigned 4 bytes per cell (L1_TO_L2*L2_MAX_MAPS*4)
 *	0x3200 l2kernel_state -- uint8_t 1 byte per cell (UNROLL_FACTOR*L2_KERNEL_SIZE)
 *	0x3234 l2kernel_scale_addr -- unsigned 4 bytes per cell (L1_TO_L2*L2_MAX_MAPS*4)
 *	0x3374 l2kernel_scale_state -- float 4 bytes per cell (UNROLL_FACTOR*4)
 *	0x3e44 mailboxes -- 4 unsigned locations 
 *	0x3e54 scratchpad -- for storing temporary intermediate results
 *	0x7000 Stack -- 4096 bytes 
 *****************************************************************************************************
 */

#include <stdint.h>
#include "e_lib.h"
#include "common.h"

volatile e_barrier_t  barriers[NUM_PES];
         e_barrier_t *tgt_bars[NUM_PES];

// global memory management -- stack start address 
asm(".global __stack_start__;");
asm(".set __stack_start__,0x6000;");
asm(".global __heap_start__;");
asm(".set __heap_start__,0x0000;");
asm(".global __heap_end__;");
asm(".set __heap_end__,0x3ffc;");

unsigned *L1_num_maps, *L2_num_maps, *l1map_addr, *mailboxes;
uint8_t *image, *L1_kernel, *L2_kernel;
uint16_t *L1_maps, *L2_maps, *l1map_state;
float *scratchpad;
float *L1_kernel_scale, *L2_kernel_scale; 

//function declarations
void normalize(float *input, unsigned input_size);
void filter2D(uint8_t *kernel, uint16_t *src, uint8_t *src1, float *dest, unsigned kernel_width, unsigned src_width, float scale_fix, int isL1);
void subsample(float *input, uint16_t *dest, unsigned input_size, unsigned factor);

#define CTIMERADD (0x000f0438)

//unsigned timer_offset;

int main(void){

	mailboxes = (unsigned *)DONE_ADDR; //4 mailbox locations available per core
	*mailboxes = 0xbeefdead;
//	mailboxes[1] = 0;

//	unsigned time_s, time_e;
//	unsigned ctimer, row, col;

	//setup timer
/*	row = e_group_config.core_row;
	col = e_group_config.core_col;
	ctimer = (unsigned)e_get_global_address(row, col, (void *)CTIMERADD);

	//record the offset
	e_ctimer_set(E_CTIMER_0, E_CTIMER_MAX);
	e_ctimer_start(E_CTIMER_0, E_CTIMER_CLK);
	__asm__ __volatile__("ldr %0, [%1]":"=r"(time_s):"r"(ctimer):);
	__asm__ __volatile__("ldr %0, [%1]":"=r"(time_e):"r"(ctimer):);
	e_ctimer_stop(E_CTIMER_0);
	timer_offset = time_s - time_e;
*/
	//initialize barriers
	e_barrier_init(barriers, tgt_bars);

	/*allocate memory based on number of nodes and edges*/
	L1_num_maps = (unsigned *)0x4000;
	L2_num_maps = (unsigned *)0x4004;
	image = (uint8_t *)BASE_ADDR;
	L1_kernel = (uint8_t *)L1_KERNEL_ADDR;
	L1_kernel_scale = (float *)L1_KERNEL_SCALE_ADDR;
	L2_kernel = (uint8_t *)L2_KERNEL_ADDR;
	L2_kernel_scale = (float *)L2_KERNEL_SCALE_ADDR;
	L1_maps = (uint16_t *)L1_MAPS_ADDR;
	L2_maps = (uint16_t *)L2_MAPS_ADDR;
	l1map_addr = (unsigned *)L2_L1_MAP_ADDR;
	l1map_state = (uint16_t *)L2_L1_MAP_STATE_ADDR;

	scratchpad = (float *)SCRATCHPAD_ADDR;

	int i,j,k;
	unsigned coreid = e_group_config.core_row*4+e_group_config.core_col;
	/*--------------- layer 1 ----------------*/
	//setup and initialize scratchpad memory for L1 compute
	unsigned conv_size = 22;

	//pointers to different temporary structures
	float *filter2D_out = (float *)(scratchpad);

//	e_ctimer_set(E_CTIMER_0, E_CTIMER_MAX);
//	e_ctimer_start(E_CTIMER_0, E_CTIMER_CLK);
//	__asm__ __volatile__("ldr %0, [%1]":"=r"(time_s):"r"(ctimer):);

	/* ----------- layer 1 ------------ */
	for (i=0;i<*L1_num_maps;i++){
		
		uint8_t *l1_kernel = (uint8_t *)(L1_kernel + (i*L1_KERNEL_SIZE));
		uint16_t *l1_map = (uint16_t *)(L1_maps + (i*L1_MAP_SIZE));
		float scale_fix = L1_kernel_scale[i];

		filter2D(l1_kernel,l1_map,image,filter2D_out,L1_KERNEL_WIDTH,IMG_WIDTH,scale_fix,1);
		normalize(filter2D_out,conv_size);
		subsample(filter2D_out,l1_map,conv_size,DOWN_FAC1);
	}

	//barrier so that L1 computes are done before L2 starts
	e_barrier(barriers, tgt_bars);

	/* ----------- layer 2 ------------ */
	unsigned in_size = 11;
	conv_size = 7;
	float *accum = (float *)(filter2D_out + conv_size*conv_size);

	for (i=0;i<*L2_num_maps;i++){
		for (j=0;j<conv_size*conv_size;j++)
			accum[j] = 0;
		
		for (j=0;j<L1_TO_L2;j+=UNROLL_FACTOR){
			uint16_t *src = (uint16_t *)(l1map_addr[i*L1_TO_L2+j]);
			uint16_t *dest = (uint16_t *)(l1map_state); 
			
			e_dma_copy(dest,src,L1_MAP_SIZE*sizeof(uint16_t));

			//this is just a dummy pointer for sanity sake
			uint8_t *k_dest = (uint8_t *)(0x4000);

			filter2D(&L2_kernel[i],l1map_state,k_dest,filter2D_out,L2_KERNEL_WIDTH,in_size,L2_kernel_scale[i],0);

			for (k=0;k<conv_size*conv_size;k++){
				accum[k] += filter2D_out[k];
			}
		}
	
		float third_scale = 0.4f;//L1_TO_L2/L2_KERNEL_SIZE;
		//transfer to filter2D_out at the same time
		for (k=0;k<conv_size*conv_size;k++){
			filter2D_out[k] = accum[k]*third_scale;
			if (filter2D_out[k] <= 0)
				filter2D_out[k] = 0;
		}

		//can't subsample by 2, since current size is 7*7, need to pad to make it 8*8
		int pad = 1;
		accum[0] = filter2D_out[0];
		for (k=0;k<conv_size;k++)
			accum[k+1] = filter2D_out[k];
		for (j=0;j<conv_size;j++){
			for (k=0;k<conv_size+pad;k++){
				if (k == 0)
					accum[(j+1)*(conv_size+pad)+k] = filter2D_out[j*conv_size];
				else
					accum[(j+1)*(conv_size+pad)+k] = filter2D_out[j*conv_size+k-1];
			}
		}

		uint16_t *l2_map = (uint16_t *)(L2_maps + (i*L2_MAP_SIZE));
		subsample(accum,l2_map,conv_size+pad,DOWN_FAC2);
	}

//	__asm__ __volatile__("ldr %0, [%1]":"=r"(time_e):"r"(ctimer):);
//	e_ctimer_stop(E_CTIMER_0);
	
//	mailboxes[1] = time_s - time_e - timer_offset;
	*mailboxes = 0xdeadbeef;

	return 0;
}

void subsample(float *input, uint16_t *dest, unsigned input_size, unsigned factor){
	int i,j;
	int cntr = 0, cntr1 = 0;
	int s = input_size/factor;
	for (i=0;i<s;i++){
		for (j=0;j<s;j++){
			*(dest + cntr) = ((uint16_t)(input[cntr1]+0.5f));
			cntr++;
			cntr1 += factor;
		}
	}
}

void normalize(float *input, unsigned input_size){
	int i;
	float max = 0.0f;
	unsigned total = input_size*input_size;
	for (i=0;i<total;i++){
		if (input[i] > max)
			max = input[i];
	}
	float scale = 255.0/max;
	for (i=0;i<total;i++)
		input[i] = (input[i]*scale);
}

void filter2D(uint8_t *kernel, uint16_t *src, uint8_t *src1, float *dest, unsigned kernel_width, unsigned src_width, float scale_fix, int isL1){
	unsigned row,col,kernel_row,kernel_col,i;
	int cntr[kernel_width];
	unsigned o_cntr = 0;
	unsigned conv_size = src_width - kernel_width + 1;
	for (row=0;row<conv_size;row++){
		for (i=0;i<kernel_width;i++)
			cntr[i] = (row+i)*src_width;
		for (col=0;col<conv_size;col++){
			float sop = 0.0f;

			for (kernel_row=0;kernel_row<kernel_width;kernel_row++){
				for (kernel_col=0;kernel_col<kernel_width;kernel_col++){
					if (isL1){
						sop += ((float)kernel[kernel_row*kernel_width+kernel_col])*((float)src1[cntr[kernel_row]+kernel_col]);
					}
					else{
						sop += ((float)kernel[kernel_row*kernel_width+kernel_col])*((float)src[cntr[kernel_row]+kernel_col]);
					}
				}
			}
			
			sop = sop*scale_fix;
			if (isL1)
				*(dest + o_cntr) = sop > 0 ? sop : 0;//non-linearity
			else 
				*(dest + o_cntr) = sop;//non-linearity

			o_cntr++;
			
			for (i=0;i<kernel_width;i++)
				cntr[i]++;
		}
	}
}
