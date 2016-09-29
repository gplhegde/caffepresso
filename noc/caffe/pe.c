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
GLOBAL_CONSTANTS_T *num_maps, *num_patches, *kernel_width, *patch_width, *patch_height;
GLOBAL_CONSTANTS_T *subsample_factor, *big_map_width;
ADDR_T *dram_kernel_ptr, *dram_map_ptr, *patch_ptr;
PATCH_T *patch;
KERNEL_T *kernel;
MAP_T *local_map_accum;
SCALE_T *kernel_scale;

//function declarations
void load_kernels(ADDR_T ptr, int n, int k, int coreid);
void reduction(MAP_T *state, int map_size);
void filter2D(KERNEL_T *kernel, MAP_T *src, INTERMEDIATE_T *dest, unsigned kernel_width, unsigned src_height, unsigned src_width, SCALE_T scale);
void subsample(INTERMEDIATE_T *src, MAP_T *dest, unsigned stride, unsigned src_height, unsigned src_width);
void dump_to_dram(ADDR_T ptr, int patch_id, unsigned mheight, unsigned mwidth, unsigned big_mwidth);
void load_patch(int patch_num, unsigned ph, unsigned pw, ADDR_T ptr);

int main(void){

	mailboxes = (FLAG_T *)DONE_ADDR; //4 mailbox locations available per core
	*mailboxes = 0xbeefdead;
	mailboxes[1] = 0;

	//initialize barriers
	e_barrier_init(barriers, tgt_bars);

	unsigned coreid = e_group_config.core_row*4+e_group_config.core_col;

	/*allocate memory based on number of nodes and edges*/
	num_maps = (GLOBAL_CONSTANTS_T *)PARAMETERS_ADDR;
	num_patches = (GLOBAL_CONSTANTS_T *)(num_maps + 1);
	kernel_width = (GLOBAL_CONSTANTS_T *)(num_patches + 1);
	patch_height = (GLOBAL_CONSTANTS_T *)(kernel_width + 1);
	patch_width = (GLOBAL_CONSTANTS_T *)(patch_height + 1);
	subsample_factor = (GLOBAL_CONSTANTS_T *)(patch_width + 1);
	dram_kernel_ptr = (ADDR_T *)(subsample_factor + 1);
	dram_map_ptr = (ADDR_T *)(dram_kernel_ptr + 1);
	patch_ptr = (ADDR_T *)(dram_map_ptr + 1);
	big_map_width = (GLOBAL_CONSTANTS_T *)(patch_ptr + 1);

	int i,j,p;
	
	//assuming kernel_width === kernel_height
	unsigned accum_map_height = (*patch_height - *kernel_width + 1)/(*subsample_factor);
	unsigned accum_map_width = (*patch_width - *kernel_width + 1)/(*subsample_factor);
	unsigned map_size = accum_map_height*accum_map_width;

	kernel = (KERNEL_T *)(big_map_width + 1);
	kernel_scale = (SCALE_T *)(kernel + (*num_maps)*(*kernel_width)*(*kernel_width));
	patch = (PATCH_T *)(kernel_scale + (*num_maps));
	local_map_accum = (MAP_T *)(patch + (*patch_height)*(*patch_width));
	INTERMEDIATE_T *filter2D_out = (INTERMEDIATE_T *)(local_map_accum + map_size);
	unsigned conv_height = accum_map_height*(*subsample_factor);
	unsigned conv_width = accum_map_width*(*subsample_factor);
	MAP_T *map_patch = (MAP_T *)(filter2D_out + conv_height*conv_width);

	load_kernels(*dram_kernel_ptr,*num_maps,*kernel_width,coreid);

	for (p=0;p<*num_patches;p++){
		load_patch(p,*patch_height,*patch_width,*patch_ptr);

		for (i=0;i<*num_maps;i++){
			KERNEL_T *ker = (KERNEL_T *)(kernel + (i*(*kernel_width)*(*kernel_width)));
			SCALE_T scale = kernel_scale[i];

			filter2D(ker,patch,filter2D_out,*kernel_width,*patch_height,*patch_width,scale);
			subsample(filter2D_out,map_patch,*subsample_factor,*patch_height,*patch_width);

			//local-accum for all L1-maps			
			if (i==0){
				for (j=0;j<map_size;j++)
					local_map_accum[j] = map_patch[j];
			} else {
				for (j=0;j<map_size;j++)
					local_map_accum[j] += map_patch[j];
			}
		}
		//synchronize all cores
		e_barrier(barriers, tgt_bars);
		
		if (coreid == 0){
			MAP_T *state = (MAP_T *)filter2D_out;
			reduction(state,map_size); //grabs local-accums from all eCores and does a global accum to generate one accum L1 map
			dump_to_dram(*dram_map_ptr,p,accum_map_height,accum_map_width,*big_map_width);
		}
		
		mailboxes[1]++;
	}
	*mailboxes = 0xdeadbeef;

	return 0;
}

void dump_to_dram(ADDR_T ptr, int patch_id, unsigned mheight, unsigned mwidth, unsigned big_mwidth){

	int i;
	unsigned ppr = big_mwidth/mwidth;
	unsigned col = patch_id%ppr;
	unsigned row = (patch_id-col)/ppr;
	ADDR_T *index = (ADDR_T *)(ptr + row*big_mwidth + col*mwidth);
	for (i=0;i<mheight;i++){
		ADDR_T *offset = (ADDR_T *)(index + i*big_mwidth);
		MAP_T *src = (MAP_T *)(local_map_accum + i*mwidth);
		e_dma_copy(offset,src,mwidth*sizeof(MAP_T));
	}
}

void reduction(MAP_T *state, int map_size){
	int i,j;

	ADDR_T *sr0 = (ADDR_T *)e_get_global_address(0,1,local_map_accum);
	ADDR_T *sr1 = (ADDR_T *)e_get_global_address(0,2,local_map_accum);
	ADDR_T *sr2 = (ADDR_T *)e_get_global_address(0,3,local_map_accum);

	MAP_T *des0 = (MAP_T *)(state); 
	MAP_T *des1 = (MAP_T *)(state + map_size);
	MAP_T *des2 = (MAP_T *)(state + 2*map_size);

	e_dma_copy(des0,sr0,map_size*sizeof(MAP_T));
	e_dma_copy(des1,sr1,map_size*sizeof(MAP_T));
	e_dma_copy(des2,sr2,map_size*sizeof(MAP_T));

	//accum
	for (j=0;j<map_size;j++){
		local_map_accum[j] += des0[j] + des1[j] + des2[j];
	}

	for (i=1;i<4;i++){;

		ADDR_T *src0 = (ADDR_T *)e_get_global_address(i,0,local_map_accum);
		ADDR_T *src1 = (ADDR_T *)e_get_global_address(i,1,local_map_accum);
		ADDR_T *src2 = (ADDR_T *)e_get_global_address(i,2,local_map_accum);
		ADDR_T *src3 = (ADDR_T *)e_get_global_address(i,3,local_map_accum);
		

		MAP_T *dest0 = (MAP_T *)(state); 
		MAP_T *dest1 = (MAP_T *)(state + map_size);
		MAP_T *dest2 = (MAP_T *)(state + 2*map_size);
		MAP_T *dest3 = (MAP_T *)(state + 3*map_size);

		e_dma_copy(dest0,src0,map_size*sizeof(MAP_T));
		e_dma_copy(dest1,src1,map_size*sizeof(MAP_T));
		e_dma_copy(dest2,src2,map_size*sizeof(MAP_T));
		e_dma_copy(dest3,src3,map_size*sizeof(MAP_T));

		//accum
		for (j=0;j<map_size;j++){
			local_map_accum[j] += dest0[j] + dest1[j] + dest2[j] + dest3[j];
		}
	}
}

void load_kernels(ADDR_T ptr, int n, int k, int coreid){
	ADDR_T *addr = (ADDR_T *)(ptr + n*coreid);
	e_dma_copy(kernel,addr,(n*k*k+n)*sizeof(KERNEL_T));
}

void load_patch(int patch_num, unsigned ph, unsigned pw , ADDR_T ptr){
	ADDR_T *addr = (ADDR_T *)(ptr + patch_num*ph*pw);
	e_dma_copy(patch,addr,ph*pw*sizeof(PATCH_T));
}

void filter2D(KERNEL_T *kernel, MAP_T *src, INTERMEDIATE_T *dest, unsigned kernel_width, 
	unsigned src_height, unsigned src_width, SCALE_T scale_fix){

	unsigned row,col,kernel_row,kernel_col,i;
	int cntr[kernel_width];
	unsigned o_cntr = 0;
	unsigned conv_height= src_height - kernel_width + 1;
	unsigned conv_width = src_width - kernel_width + 1;
	for (row=0;row<conv_height;row++){
		for (i=0;i<kernel_width;i++)
			cntr[i] = (row+i)*src_width;
		for (col=0;col<conv_width;col++){
			INTERMEDIATE_T sop = 0.0f;

			for (kernel_row=0;kernel_row<kernel_width;kernel_row++){
				for (kernel_col=0;kernel_col<kernel_width;kernel_col++){
					sop += kernel[kernel_row*kernel_width+kernel_col]*src[cntr[kernel_row]+kernel_col];			
				}
			}
			
			sop = sop*scale_fix;
			*(dest + o_cntr) = sop;

			o_cntr++;
			
			for (i=0;i<kernel_width;i++)
				cntr[i]++;
		}
	}
}

void subsample(INTERMEDIATE_T *src, MAP_T *dest, unsigned stride, unsigned src_height, unsigned src_width){
	unsigned row,col,i,j;
	int cntr[stride];
	unsigned o_cntr = 0;

	unsigned corr_height = src_height - src_height%stride;
	unsigned corr_width = src_width - src_width%stride;

	for (row=0;row<corr_height;row+=stride){
		for (i=0;i<stride;i++)
			cntr[i] = (row+i)*src_width;
		for (col=0;col<corr_width;col+=stride){

			//2x2 unrolled max-pool
			float c0,c1;
			c0 = src[cntr[0]] > src[cntr[0]+1] ? src[cntr[0]] : src[cntr[0]+1];
			c1 = src[cntr[1]] > src[cntr[1]+1] ? src[cntr[1]] : src[cntr[1]+1];

			c0 = c0 > c1 ? c0 : c1;

			dest[o_cntr] = c0;
			o_cntr++;

			for (i=0;i<stride;i++)
				cntr[i] += stride;
		}
	}
}
