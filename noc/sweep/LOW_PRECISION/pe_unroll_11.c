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
		
			sop += ((INTERMEDIATE_T)kernel[0])*((INTERMEDIATE_T)src[cntr[0]]);
			sop += ((INTERMEDIATE_T)kernel[1])*((INTERMEDIATE_T)src[cntr[0]+1]);
			sop += ((INTERMEDIATE_T)kernel[2])*((INTERMEDIATE_T)src[cntr[0]+2]);
			sop += ((INTERMEDIATE_T)kernel[3])*((INTERMEDIATE_T)src[cntr[0]+3]);
			sop += ((INTERMEDIATE_T)kernel[4])*((INTERMEDIATE_T)src[cntr[0]+4]);
			sop += ((INTERMEDIATE_T)kernel[5])*((INTERMEDIATE_T)src[cntr[0]+5]);
			sop += ((INTERMEDIATE_T)kernel[6])*((INTERMEDIATE_T)src[cntr[0]+6]);
			sop += ((INTERMEDIATE_T)kernel[7])*((INTERMEDIATE_T)src[cntr[0]+7]);
			sop += ((INTERMEDIATE_T)kernel[8])*((INTERMEDIATE_T)src[cntr[0]+8]);
			sop += ((INTERMEDIATE_T)kernel[9])*((INTERMEDIATE_T)src[cntr[0]+9]);
			sop += ((INTERMEDIATE_T)kernel[10])*((INTERMEDIATE_T)src[cntr[0]+10]);

			sop += ((INTERMEDIATE_T)kernel[11])*((INTERMEDIATE_T)src[cntr[1]]);
			sop += ((INTERMEDIATE_T)kernel[12])*((INTERMEDIATE_T)src[cntr[1]+1]);
			sop += ((INTERMEDIATE_T)kernel[13])*((INTERMEDIATE_T)src[cntr[1]+2]);
			sop += ((INTERMEDIATE_T)kernel[14])*((INTERMEDIATE_T)src[cntr[1]+3]);
			sop += ((INTERMEDIATE_T)kernel[15])*((INTERMEDIATE_T)src[cntr[1]+4]);
			sop += ((INTERMEDIATE_T)kernel[16])*((INTERMEDIATE_T)src[cntr[1]+5]);
			sop += ((INTERMEDIATE_T)kernel[17])*((INTERMEDIATE_T)src[cntr[1]+6]);
			sop += ((INTERMEDIATE_T)kernel[18])*((INTERMEDIATE_T)src[cntr[1]+7]);
			sop += ((INTERMEDIATE_T)kernel[19])*((INTERMEDIATE_T)src[cntr[1]+8]);
			sop += ((INTERMEDIATE_T)kernel[20])*((INTERMEDIATE_T)src[cntr[1]+9]);
			sop += ((INTERMEDIATE_T)kernel[21])*((INTERMEDIATE_T)src[cntr[1]+10]);

			sop += ((INTERMEDIATE_T)kernel[22])*((INTERMEDIATE_T)src[cntr[2]]);
			sop += ((INTERMEDIATE_T)kernel[23])*((INTERMEDIATE_T)src[cntr[2]+1]);
			sop += ((INTERMEDIATE_T)kernel[24])*((INTERMEDIATE_T)src[cntr[2]+2]);
			sop += ((INTERMEDIATE_T)kernel[25])*((INTERMEDIATE_T)src[cntr[2]+3]);
			sop += ((INTERMEDIATE_T)kernel[26])*((INTERMEDIATE_T)src[cntr[2]+4]);
			sop += ((INTERMEDIATE_T)kernel[27])*((INTERMEDIATE_T)src[cntr[2]+5]);
			sop += ((INTERMEDIATE_T)kernel[28])*((INTERMEDIATE_T)src[cntr[2]+6]);
			sop += ((INTERMEDIATE_T)kernel[29])*((INTERMEDIATE_T)src[cntr[2]+7]);
			sop += ((INTERMEDIATE_T)kernel[30])*((INTERMEDIATE_T)src[cntr[2]+8]);
			sop += ((INTERMEDIATE_T)kernel[31])*((INTERMEDIATE_T)src[cntr[2]+9]);
			sop += ((INTERMEDIATE_T)kernel[32])*((INTERMEDIATE_T)src[cntr[2]+10]);

			sop += ((INTERMEDIATE_T)kernel[33])*((INTERMEDIATE_T)src[cntr[3]]);
			sop += ((INTERMEDIATE_T)kernel[34])*((INTERMEDIATE_T)src[cntr[3]+1]);
			sop += ((INTERMEDIATE_T)kernel[35])*((INTERMEDIATE_T)src[cntr[3]+2]);
			sop += ((INTERMEDIATE_T)kernel[36])*((INTERMEDIATE_T)src[cntr[3]+3]);
			sop += ((INTERMEDIATE_T)kernel[37])*((INTERMEDIATE_T)src[cntr[3]+4]);
			sop += ((INTERMEDIATE_T)kernel[38])*((INTERMEDIATE_T)src[cntr[3]+5]);
			sop += ((INTERMEDIATE_T)kernel[39])*((INTERMEDIATE_T)src[cntr[3]+6]);
			sop += ((INTERMEDIATE_T)kernel[40])*((INTERMEDIATE_T)src[cntr[3]+7]);
			sop += ((INTERMEDIATE_T)kernel[41])*((INTERMEDIATE_T)src[cntr[3]+8]);
			sop += ((INTERMEDIATE_T)kernel[42])*((INTERMEDIATE_T)src[cntr[3]+9]);
			sop += ((INTERMEDIATE_T)kernel[43])*((INTERMEDIATE_T)src[cntr[3]+10]);

			sop += ((INTERMEDIATE_T)kernel[44])*((INTERMEDIATE_T)src[cntr[4]]);
			sop += ((INTERMEDIATE_T)kernel[45])*((INTERMEDIATE_T)src[cntr[4]+1]);
			sop += ((INTERMEDIATE_T)kernel[46])*((INTERMEDIATE_T)src[cntr[4]+2]);
			sop += ((INTERMEDIATE_T)kernel[47])*((INTERMEDIATE_T)src[cntr[4]+3]);
			sop += ((INTERMEDIATE_T)kernel[48])*((INTERMEDIATE_T)src[cntr[4]+4]);
			sop += ((INTERMEDIATE_T)kernel[49])*((INTERMEDIATE_T)src[cntr[4]+5]);
			sop += ((INTERMEDIATE_T)kernel[50])*((INTERMEDIATE_T)src[cntr[4]+6]);
			sop += ((INTERMEDIATE_T)kernel[51])*((INTERMEDIATE_T)src[cntr[4]+7]);
			sop += ((INTERMEDIATE_T)kernel[52])*((INTERMEDIATE_T)src[cntr[4]+8]);
			sop += ((INTERMEDIATE_T)kernel[53])*((INTERMEDIATE_T)src[cntr[4]+9]);
			sop += ((INTERMEDIATE_T)kernel[54])*((INTERMEDIATE_T)src[cntr[4]+10]);

			sop += ((INTERMEDIATE_T)kernel[55])*((INTERMEDIATE_T)src[cntr[5]]);
			sop += ((INTERMEDIATE_T)kernel[56])*((INTERMEDIATE_T)src[cntr[5]+1]);
			sop += ((INTERMEDIATE_T)kernel[57])*((INTERMEDIATE_T)src[cntr[5]+2]);
			sop += ((INTERMEDIATE_T)kernel[58])*((INTERMEDIATE_T)src[cntr[5]+3]);
			sop += ((INTERMEDIATE_T)kernel[59])*((INTERMEDIATE_T)src[cntr[5]+4]);
			sop += ((INTERMEDIATE_T)kernel[60])*((INTERMEDIATE_T)src[cntr[5]+5]);
			sop += ((INTERMEDIATE_T)kernel[61])*((INTERMEDIATE_T)src[cntr[5]+6]);
			sop += ((INTERMEDIATE_T)kernel[62])*((INTERMEDIATE_T)src[cntr[5]+7]);
			sop += ((INTERMEDIATE_T)kernel[63])*((INTERMEDIATE_T)src[cntr[5]+8]);
			sop += ((INTERMEDIATE_T)kernel[64])*((INTERMEDIATE_T)src[cntr[5]+9]);
			sop += ((INTERMEDIATE_T)kernel[65])*((INTERMEDIATE_T)src[cntr[5]+10]);

			sop += ((INTERMEDIATE_T)kernel[66])*((INTERMEDIATE_T)src[cntr[6]]);
			sop += ((INTERMEDIATE_T)kernel[67])*((INTERMEDIATE_T)src[cntr[6]+1]);
			sop += ((INTERMEDIATE_T)kernel[68])*((INTERMEDIATE_T)src[cntr[6]+2]);
			sop += ((INTERMEDIATE_T)kernel[69])*((INTERMEDIATE_T)src[cntr[6]+3]);
			sop += ((INTERMEDIATE_T)kernel[70])*((INTERMEDIATE_T)src[cntr[6]+4]);
			sop += ((INTERMEDIATE_T)kernel[71])*((INTERMEDIATE_T)src[cntr[6]+5]);
			sop += ((INTERMEDIATE_T)kernel[72])*((INTERMEDIATE_T)src[cntr[6]+6]);
			sop += ((INTERMEDIATE_T)kernel[73])*((INTERMEDIATE_T)src[cntr[6]+7]);
			sop += ((INTERMEDIATE_T)kernel[74])*((INTERMEDIATE_T)src[cntr[6]+8]);
			sop += ((INTERMEDIATE_T)kernel[75])*((INTERMEDIATE_T)src[cntr[6]+9]);
			sop += ((INTERMEDIATE_T)kernel[76])*((INTERMEDIATE_T)src[cntr[6]+10]);

			sop += ((INTERMEDIATE_T)kernel[77])*((INTERMEDIATE_T)src[cntr[7]]);
			sop += ((INTERMEDIATE_T)kernel[78])*((INTERMEDIATE_T)src[cntr[7]+1]);
			sop += ((INTERMEDIATE_T)kernel[79])*((INTERMEDIATE_T)src[cntr[7]+2]);
			sop += ((INTERMEDIATE_T)kernel[80])*((INTERMEDIATE_T)src[cntr[7]+3]);
			sop += ((INTERMEDIATE_T)kernel[81])*((INTERMEDIATE_T)src[cntr[7]+4]);
			sop += ((INTERMEDIATE_T)kernel[82])*((INTERMEDIATE_T)src[cntr[7]+5]);
			sop += ((INTERMEDIATE_T)kernel[83])*((INTERMEDIATE_T)src[cntr[7]+6]);
			sop += ((INTERMEDIATE_T)kernel[84])*((INTERMEDIATE_T)src[cntr[7]+7]);
			sop += ((INTERMEDIATE_T)kernel[85])*((INTERMEDIATE_T)src[cntr[7]+8]);
			sop += ((INTERMEDIATE_T)kernel[86])*((INTERMEDIATE_T)src[cntr[7]+9]);
			sop += ((INTERMEDIATE_T)kernel[87])*((INTERMEDIATE_T)src[cntr[7]+10]);

			sop += ((INTERMEDIATE_T)kernel[88])*((INTERMEDIATE_T)src[cntr[8]]);
			sop += ((INTERMEDIATE_T)kernel[89])*((INTERMEDIATE_T)src[cntr[8]+1]);
			sop += ((INTERMEDIATE_T)kernel[90])*((INTERMEDIATE_T)src[cntr[8]+2]);
			sop += ((INTERMEDIATE_T)kernel[91])*((INTERMEDIATE_T)src[cntr[8]+3]);
			sop += ((INTERMEDIATE_T)kernel[92])*((INTERMEDIATE_T)src[cntr[8]+4]);
			sop += ((INTERMEDIATE_T)kernel[93])*((INTERMEDIATE_T)src[cntr[8]+5]);
			sop += ((INTERMEDIATE_T)kernel[94])*((INTERMEDIATE_T)src[cntr[8]+6]);
			sop += ((INTERMEDIATE_T)kernel[95])*((INTERMEDIATE_T)src[cntr[8]+7]);
			sop += ((INTERMEDIATE_T)kernel[96])*((INTERMEDIATE_T)src[cntr[8]+8]);
			sop += ((INTERMEDIATE_T)kernel[97])*((INTERMEDIATE_T)src[cntr[8]+9]);
			sop += ((INTERMEDIATE_T)kernel[98])*((INTERMEDIATE_T)src[cntr[8]+10]);

			sop += ((INTERMEDIATE_T)kernel[99])*((INTERMEDIATE_T)src[cntr[9]]);
			sop += ((INTERMEDIATE_T)kernel[100])*((INTERMEDIATE_T)src[cntr[9]+1]);
			sop += ((INTERMEDIATE_T)kernel[101])*((INTERMEDIATE_T)src[cntr[9]+2]);
			sop += ((INTERMEDIATE_T)kernel[102])*((INTERMEDIATE_T)src[cntr[9]+3]);
			sop += ((INTERMEDIATE_T)kernel[103])*((INTERMEDIATE_T)src[cntr[9]+4]);
			sop += ((INTERMEDIATE_T)kernel[104])*((INTERMEDIATE_T)src[cntr[9]+5]);
			sop += ((INTERMEDIATE_T)kernel[105])*((INTERMEDIATE_T)src[cntr[9]+6]);
			sop += ((INTERMEDIATE_T)kernel[106])*((INTERMEDIATE_T)src[cntr[9]+7]);
			sop += ((INTERMEDIATE_T)kernel[107])*((INTERMEDIATE_T)src[cntr[9]+8]);
			sop += ((INTERMEDIATE_T)kernel[108])*((INTERMEDIATE_T)src[cntr[9]+9]);
			sop += ((INTERMEDIATE_T)kernel[109])*((INTERMEDIATE_T)src[cntr[9]+10]);

			sop += ((INTERMEDIATE_T)kernel[110])*((INTERMEDIATE_T)src[cntr[10]]);
			sop += ((INTERMEDIATE_T)kernel[111])*((INTERMEDIATE_T)src[cntr[10]+1]);
			sop += ((INTERMEDIATE_T)kernel[112])*((INTERMEDIATE_T)src[cntr[10]+2]);
			sop += ((INTERMEDIATE_T)kernel[113])*((INTERMEDIATE_T)src[cntr[10]+3]);
			sop += ((INTERMEDIATE_T)kernel[114])*((INTERMEDIATE_T)src[cntr[10]+4]);
			sop += ((INTERMEDIATE_T)kernel[115])*((INTERMEDIATE_T)src[cntr[10]+5]);
			sop += ((INTERMEDIATE_T)kernel[116])*((INTERMEDIATE_T)src[cntr[10]+6]);
			sop += ((INTERMEDIATE_T)kernel[117])*((INTERMEDIATE_T)src[cntr[10]+7]);
			sop += ((INTERMEDIATE_T)kernel[118])*((INTERMEDIATE_T)src[cntr[10]+8]);
			sop += ((INTERMEDIATE_T)kernel[119])*((INTERMEDIATE_T)src[cntr[10]+9]);
			sop += ((INTERMEDIATE_T)kernel[120])*((INTERMEDIATE_T)src[cntr[10]+10]);

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

			dest[o_cntr] = ((MAP_T)sum+0.5f);
			o_cntr++;

			for (i=0;i<stride;i++)
				cntr[i] += stride;
		}
	}
}
