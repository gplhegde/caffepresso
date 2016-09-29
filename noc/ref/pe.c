#include <stdint.h>
#include "e_lib.h"
#include "stdlib.h"
#include "string.h"
#include <e_coreid.h>

// global memory management -- stack start address, giving 8KB of stack
asm(".global __stack_start__;");
asm(".set __stack_start__,0x7000;");
asm(".global __heap_start__;");
asm(".set __heap_start__,0x0000;");
asm(".global __heap_end__;");
asm(".set __heap_end__,0x1fff;");

int main()
{

	float* kernel 		= (float*)0x2000;	// kernel coefficients
	float* image  		= (float*)0x4000; 	// input image buffer
	float* dest_image	= (float*)0x6000; 	// result image buffer

	unsigned* done = (unsigned*)0x7000;
	*done=0;

	unsigned* clocks = (unsigned*)0x7004;
	*clocks=0;

	unsigned* NUM_ROW = (unsigned*)0x7100;
	unsigned* NUM_COL = (unsigned*)0x7104;
	unsigned* NUM_KERNEL_ROW = (unsigned*)0x7108;
	unsigned* NUM_KERNEL_COL = (unsigned*)0x710C;
	int* scale_fix = (int*)0x7110;

//	unsigned time_e, time_s;
	
//	e_ctimer_set(E_CTIMER_0, E_CTIMER_MAX);					
//	e_ctimer_start(E_CTIMER_0, E_CTIMER_CLK);

//	time_e = e_ctimer_set(E_CTIMER_0, E_CTIMER_MAX);
//	time_s = e_ctimer_get(E_CTIMER_0);
//	int diff=time_e-time_s;
	
	// Filter loop
	int row, col, kernel_row, kernel_col;
	int cntr, cntr1, cntr2, offset; // removing multiplications from addrgen
	//uint16_t sop;
	float sop;

//	time_e	= e_ctimer_set(E_CTIMER_0, E_CTIMER_MAX);
	cntr=0;cntr1=*NUM_COL;cntr2=(*NUM_COL)*2;
	for (row = 0; row < *NUM_ROW; row++)
	{
		for (col = 0; col < *NUM_COL; col++)
		{
			offset = 0; sop=0;
/*
			for ( kernel_row = 0; kernel_row < *NUM_KERNEL_ROW; kernel_row ++)
			{
				for (kernel_col = 0; kernel_col < *NUM_KERNEL_COL; kernel_col++)
				{
					sop += *(kernel + offset) * 
						(*(image + cntr + offset));
					offset++;
				}
			}
*/
			// manually unrolled kernel multiplications
			sop = kernel[0]*image[cntr]+
				kernel[1]*image[cntr+1]+
				kernel[2]*image[cntr+2]+
				kernel[3]*image[cntr1]+
				kernel[4]*image[cntr1+1]+
				kernel[5]*image[cntr1+2]+
				kernel[6]*image[cntr2]+
				kernel[7]*image[cntr2+1]+
				kernel[8]*image[cntr2+2];
			// TODO: right shift properly
			// sop = (sop * (*scale_fix))>> 8;
			*(dest_image + cntr) = (sop * (*scale_fix));

			cntr++;
			cntr1++;
			cntr2++;
		}
	}
//	time_s = e_ctimer_get(E_CTIMER_0);
//	*clocks += (time_e - time_s)-diff;
				
//	e_ctimer_stop(E_CTIMER_0);					
	*done = 1;
	return 0;
}
