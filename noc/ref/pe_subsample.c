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

	unsigned time_e, time_s;
	
	e_ctimer_set(E_CTIMER_0, E_CTIMER_MAX);					
	e_ctimer_start(E_CTIMER_0, E_CTIMER_CLK);

	time_e = e_ctimer_set(E_CTIMER_0, E_CTIMER_MAX);
	time_s = e_ctimer_get(E_CTIMER_0);
	int diff=time_e-time_s;
	
	// Filter loop
	int row, col;
	int cntr1=0, cntr2=0; // removing multiplications from addrgen

	time_e	= e_ctimer_set(E_CTIMER_0, E_CTIMER_MAX);
	for (row = 0; row < *NUM_ROW/2; row++)
	{
		for (col = 0; col < *NUM_COL/2; col++)
		{
			*(dest_image + cntr1) = *(image + cntr2);
			cntr1++;
			cntr2+=2;
		}
		cntr2+=*NUM_COL;
	}

	// TODO: filter2D call
	// int lin_int_kernel[9] = {1,2,1,2,4,2,1,2,1};

	time_s = e_ctimer_get(E_CTIMER_0);
	*clocks += (time_e - time_s)-diff;
				
	e_ctimer_stop(E_CTIMER_0);					
	*done = 1;
	return 0;
}
