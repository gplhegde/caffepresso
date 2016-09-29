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
			sop += (kernel[5])*(src[cntr[0]+5]);
			sop += (kernel[6])*(src[cntr[0]+6]);
			sop += (kernel[7])*(src[cntr[0]+7]);
			sop += (kernel[8])*(src[cntr[0]+8]);
			sop += (kernel[9])*(src[cntr[0]+9]);
			sop += (kernel[10])*(src[cntr[0]+10]);

			sop += (kernel[11])*(src[cntr[1]]);
			sop += (kernel[12])*(src[cntr[1]+1]);
			sop += (kernel[13])*(src[cntr[1]+2]);
			sop += (kernel[14])*(src[cntr[1]+3]);
			sop += (kernel[15])*(src[cntr[1]+4]);
			sop += (kernel[16])*(src[cntr[1]+5]);
			sop += (kernel[17])*(src[cntr[1]+6]);
			sop += (kernel[18])*(src[cntr[1]+7]);
			sop += (kernel[19])*(src[cntr[1]+8]);
			sop += (kernel[20])*(src[cntr[1]+9]);
			sop += (kernel[21])*(src[cntr[1]+10]);

			sop += (kernel[22])*(src[cntr[2]]);
			sop += (kernel[23])*(src[cntr[2]+1]);
			sop += (kernel[24])*(src[cntr[2]+2]);
			sop += (kernel[25])*(src[cntr[2]+3]);
			sop += (kernel[26])*(src[cntr[2]+4]);
			sop += (kernel[27])*(src[cntr[2]+5]);
			sop += (kernel[28])*(src[cntr[2]+6]);
			sop += (kernel[29])*(src[cntr[2]+7]);
			sop += (kernel[30])*(src[cntr[2]+8]);
			sop += (kernel[31])*(src[cntr[2]+9]);
			sop += (kernel[32])*(src[cntr[2]+10]);

			sop += (kernel[33])*(src[cntr[3]]);
			sop += (kernel[34])*(src[cntr[3]+1]);
			sop += (kernel[35])*(src[cntr[3]+2]);
			sop += (kernel[36])*(src[cntr[3]+3]);
			sop += (kernel[37])*(src[cntr[3]+4]);
			sop += (kernel[38])*(src[cntr[3]+5]);
			sop += (kernel[39])*(src[cntr[3]+6]);
			sop += (kernel[40])*(src[cntr[3]+7]);
			sop += (kernel[41])*(src[cntr[3]+8]);
			sop += (kernel[42])*(src[cntr[3]+9]);
			sop += (kernel[43])*(src[cntr[3]+10]);

			sop += (kernel[44])*(src[cntr[4]]);
			sop += (kernel[45])*(src[cntr[4]+1]);
			sop += (kernel[46])*(src[cntr[4]+2]);
			sop += (kernel[47])*(src[cntr[4]+3]);
			sop += (kernel[48])*(src[cntr[4]+4]);
			sop += (kernel[49])*(src[cntr[4]+5]);
			sop += (kernel[50])*(src[cntr[4]+6]);
			sop += (kernel[51])*(src[cntr[4]+7]);
			sop += (kernel[52])*(src[cntr[4]+8]);
			sop += (kernel[53])*(src[cntr[4]+9]);
			sop += (kernel[54])*(src[cntr[4]+10]);

			sop += (kernel[55])*(src[cntr[5]]);
			sop += (kernel[56])*(src[cntr[5]+1]);
			sop += (kernel[57])*(src[cntr[5]+2]);
			sop += (kernel[58])*(src[cntr[5]+3]);
			sop += (kernel[59])*(src[cntr[5]+4]);
			sop += (kernel[60])*(src[cntr[5]+5]);
			sop += (kernel[61])*(src[cntr[5]+6]);
			sop += (kernel[62])*(src[cntr[5]+7]);
			sop += (kernel[63])*(src[cntr[5]+8]);
			sop += (kernel[64])*(src[cntr[5]+9]);
			sop += (kernel[65])*(src[cntr[5]+10]);

			sop += (kernel[66])*(src[cntr[6]]);
			sop += (kernel[67])*(src[cntr[6]+1]);
			sop += (kernel[68])*(src[cntr[6]+2]);
			sop += (kernel[69])*(src[cntr[6]+3]);
			sop += (kernel[70])*(src[cntr[6]+4]);
			sop += (kernel[71])*(src[cntr[6]+5]);
			sop += (kernel[72])*(src[cntr[6]+6]);
			sop += (kernel[73])*(src[cntr[6]+7]);
			sop += (kernel[74])*(src[cntr[6]+8]);
			sop += (kernel[75])*(src[cntr[6]+9]);
			sop += (kernel[76])*(src[cntr[6]+10]);

			sop += (kernel[77])*(src[cntr[7]]);
			sop += (kernel[78])*(src[cntr[7]+1]);
			sop += (kernel[79])*(src[cntr[7]+2]);
			sop += (kernel[80])*(src[cntr[7]+3]);
			sop += (kernel[81])*(src[cntr[7]+4]);
			sop += (kernel[82])*(src[cntr[7]+5]);
			sop += (kernel[83])*(src[cntr[7]+6]);
			sop += (kernel[84])*(src[cntr[7]+7]);
			sop += (kernel[85])*(src[cntr[7]+8]);
			sop += (kernel[86])*(src[cntr[7]+9]);
			sop += (kernel[87])*(src[cntr[7]+10]);

			sop += (kernel[88])*(src[cntr[8]]);
			sop += (kernel[89])*(src[cntr[8]+1]);
			sop += (kernel[90])*(src[cntr[8]+2]);
			sop += (kernel[91])*(src[cntr[8]+3]);
			sop += (kernel[92])*(src[cntr[8]+4]);
			sop += (kernel[93])*(src[cntr[8]+5]);
			sop += (kernel[94])*(src[cntr[8]+6]);
			sop += (kernel[95])*(src[cntr[8]+7]);
			sop += (kernel[96])*(src[cntr[8]+8]);
			sop += (kernel[97])*(src[cntr[8]+9]);
			sop += (kernel[98])*(src[cntr[8]+10]);

			sop += (kernel[99])*(src[cntr[9]]);
			sop += (kernel[100])*(src[cntr[9]+1]);
			sop += (kernel[101])*(src[cntr[9]+2]);
			sop += (kernel[102])*(src[cntr[9]+3]);
			sop += (kernel[103])*(src[cntr[9]+4]);
			sop += (kernel[104])*(src[cntr[9]+5]);
			sop += (kernel[105])*(src[cntr[9]+6]);
			sop += (kernel[106])*(src[cntr[9]+7]);
			sop += (kernel[107])*(src[cntr[9]+8]);
			sop += (kernel[108])*(src[cntr[9]+9]);
			sop += (kernel[109])*(src[cntr[9]+10]);

			sop += (kernel[110])*(src[cntr[10]]);
			sop += (kernel[111])*(src[cntr[10]+1]);
			sop += (kernel[112])*(src[cntr[10]+2]);
			sop += (kernel[113])*(src[cntr[10]+3]);
			sop += (kernel[114])*(src[cntr[10]+4]);
			sop += (kernel[115])*(src[cntr[10]+5]);
			sop += (kernel[116])*(src[cntr[10]+6]);
			sop += (kernel[117])*(src[cntr[10]+7]);
			sop += (kernel[118])*(src[cntr[10]+8]);
			sop += (kernel[119])*(src[cntr[10]+9]);
			sop += (kernel[120])*(src[cntr[10]+10]);

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
