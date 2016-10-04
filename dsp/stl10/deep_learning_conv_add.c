/*******************************************************
* File Name -- deep_learn.c
*
* Authors:  Nachiappan
*
* Created on : 27-September-2015
*
* Contents: Deeplearn algorithm implementation
*           for TI TMS320C6678 - DSP
*
********************************************************/
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "user_config.h"
#include "deep_learning.h"
#include <csl_tsc.h>
#include "c6x.h"
#include <csl_cacheAux.h>

#include "imglib.h"
#include "DSP_maxval.h"
#include "DSP_add16.h"

#include "inc/kernel_5x5.h"
#include "inc/kernel_7x7.h"

#include <ti/csl/csl_semAux.h>

#define FULLY_CONNECTED

#define MIN_DSPMAX_ELEMENTS 8

extern unsigned int core_id;

#pragma DATA_SECTION(dilate, ".local_ram")
#pragma DATA_ALIGN(dilate,8)
short dilate[MIN_DSPMAX_ELEMENTS] = {0};

#pragma DATA_SECTION(local_l1_maps_ptr,".local_ram")
static short* local_l1_maps_ptr;

#pragma DATA_SECTION(local_l2_maps_ptr,".local_ram")
static short* local_l2_maps_ptr;

#pragma DATA_SECTION(temp_img_ptr,".local_ram")
static short* temp_img_ptr;

#pragma DATA_SECTION(layer1_ptr,".local_ram")
static short* layer1_ptr;

#pragma DATA_SECTION(temp1_ptr,".local_ram")
static short* temp1_ptr;

#pragma DATA_SECTION(temp2_ptr,".local_ram")
static short* temp2_ptr;

#pragma DATA_SECTION(l1_size,".local_ram")
static int l1_size;

#pragma DATA_SECTION(l2_size,".local_ram")
static int l2_size;

typedef struct alloc_info {
char *free_ptr;
int block_count;
int max_blocks;
int block_size;
int max_size;
}alloc_info;


void* mem_alloc(alloc_info* info,int size)
{
	char* temp;
	if(size > info->max_size)
		return NULL;
	if(size % info->block_size !=0)
	{
		size += info->block_size-(size % info->block_size);
	}
	if((info->block_count + (size/info->block_size)) >= info->max_blocks)
	{
		return NULL;
	}
	else
	{
		temp = info->free_ptr;
		info->free_ptr = info->free_ptr+size;
		info->block_count += (size/info->block_size);
		return temp;
	}
}

void MemMgr_HeapInit(uint32_t img_width, uint32_t img_height)
{
	int num_core,l1_width,l1_height;
	int l2_width,l2_height;
	alloc_info msmc_info,l2_info;
	int locmem_size;

	l1_width = img_width/L1_DOWN_SIZE_FACTOR;
	l1_height = img_height/L1_DOWN_SIZE_FACTOR;
	l2_width = l1_width/L2_DOWN_SIZE_FACTOR;
	l2_height = l1_height/L2_DOWN_SIZE_FACTOR;

	l1_size = l1_width*l1_height*sizeof(short);
	l2_size = l2_width*l2_height*sizeof(short);

	if(DNUM==MASTER_CORE_ID)
	{
		msmc_info.free_ptr = (char*)MSMC_REG_BASE;
		msmc_info.block_count = 0;
		msmc_info.block_size  = CACHE_L2_LINESIZE;
		msmc_info.max_size    = MSMC_SRAM_SIZE;
		msmc_info.max_blocks  = MSMC_SRAM_SIZE/CACHE_L2_LINESIZE;

		memset((void*)MSMC_REG_BASE,0x0,MSMC_SRAM_SIZE);

		for(num_core=0; num_core<NUM_CORES;num_core++)
		{
			maps_info_ptr->l1_maps_ptr[num_core] = mem_alloc(&msmc_info,l1_width*l1_height*maps_info_ptr->l1_maps[num_core]*sizeof(short));
			while(maps_info_ptr->l1_maps_ptr[num_core]==NULL);
		}
		for(num_core=0; num_core<NUM_CORES;num_core++)
		{
			maps_info_ptr->l2_maps_ptr[num_core] = mem_alloc(&msmc_info,l2_width*l2_height*maps_info_ptr->l2_maps[num_core]*sizeof(short));
			while(maps_info_ptr->l2_maps_ptr[num_core]==NULL);
		}
	    while ((CSL_semAcquireDirect (LAYER_1_SEM)) == 0);

	    /* The core has completed local initialization */
	    L1_SYNC = 0;

	    /* Release the hardware semaphore. */
	    CSL_semReleaseSemaphore ( LAYER_1_SEM);

	    while ((CSL_semAcquireDirect (LAYER_2_SEM)) == 0);

	    /* The core has completed local initialization */
	    L2_SYNC = 0;

	    /* Release the hardware semaphore. */
	    CSL_semReleaseSemaphore ( LAYER_2_SEM);

		INIT_DONE = 0x1;
	}

	while(INIT_DONE==0x0);

	l2_info.free_ptr = (char*)L2_HEAP_BASE;
	l2_info.block_count = 0;
	l2_info.block_size  = CACHE_L1D_LINESIZE;
	l2_info.max_size    = L2_HEAP_SIZE;
	l2_info.max_blocks  = L2_HEAP_SIZE/CACHE_L1D_LINESIZE;

	memset((void*)L2_HEAP_BASE,0x0,L2_HEAP_SIZE);

	locmem_size = (img_width*img_height*sizeof(short))+(3*l1_size);
	if(locmem_size>L2_HEAP_SIZE)
	{
		printf("Insufficient l2 memory \n");
		while(1);
	}

	temp_img_ptr 	= mem_alloc(&l2_info,img_width*img_height*sizeof(short));
	layer1_ptr  	= mem_alloc(&l2_info,l1_width*l1_height*sizeof(short));
	temp1_ptr  		= mem_alloc(&l2_info,l1_width*l1_height*sizeof(short));
	temp2_ptr  		= mem_alloc(&l2_info,l1_width*l1_height*sizeof(short));

	local_l1_maps_ptr= (short*)maps_info_ptr->l1_maps_ptr[DNUM];
	local_l2_maps_ptr= (short*)maps_info_ptr->l2_maps_ptr[DNUM];
}
static void maxpool(short *image,short *dest_image, int M, int N)
{
	int krows =2,kcols =2,stride_len=2;
    int row, col,kernel_row, kernel_col;
    int dest_col = N/2,drows=0,dcols=0;

    for (row = 0; row < M; row=row+stride_len,drows++)
    {
        for (col = 0; col < N; col=col+stride_len,dcols++)
        {
            for (kernel_row = 0; kernel_row < krows; kernel_row++)
            {
                for (kernel_col = 0; kernel_col < kcols; kernel_col++)
                {
                    (*(dilate + kcols*kernel_row + kernel_col)) = *(image + N*row + col + N*kernel_row + kernel_col);
                }
            }
        	*(dest_image + dest_col*drows + dcols) = DSP_maxval(&dilate[0],MIN_DSPMAX_ELEMENTS);
        }
    }

}

Conv7x7(short* src_ptr,short* out_ptr,uint32_t w,uint32_t h,const short*mask_ptr)
{
    short shift = 0; /*TODO*/
    int col;

    for(col=0;col<h-6;col++)
    {
   	 IMG_conv_7x7_i16s_c16s ((src_ptr+col*w), (out_ptr+col*w), w, w, mask_ptr,shift);
    }
}

operateLayer1(short* img_ptr, uint32_t w, uint32_t h)
{
    uint32_t i;
    uint32_t j;
    short pixels = w*h;
    short kern_index;
    uint32_t local_l1_maps = maps_info_ptr->l1_maps[DNUM];

    for (i = 0; i<local_l1_maps; i++)
    {
    	if(DNUM<maps_info_ptr->mod_l1)
    		kern_index = (DNUM*local_l1_maps)+i;
    	else
    		kern_index = (DNUM*local_l1_maps)+i+maps_info_ptr->mod_l1;

    	Conv7x7(img_ptr,temp_img_ptr,w,h,&kernel7x7[kern_index][0]);

    	for (j=0; j<pixels; j++)
        {
    		temp_img_ptr[j] = ((temp_img_ptr[j] < 0) ? 0 : temp_img_ptr[j]);
        }
        maxpool(temp_img_ptr,(local_l1_maps_ptr+(i*l1_size)), w, h);
    }
}

Conv5x5(short* src_ptr,short* out_ptr,uint32_t w,uint32_t h,const short*mask_ptr)
{
    short shift = 0; /*TODO*/
    int col;

    for(col=0;col<h-4;col++)
    {
    	IMG_conv_5x5_i16s_c16s ((src_ptr+col*w), (out_ptr+col*w), w, w, mask_ptr,shift);
    }
}

operateLayer2(uint32_t w, uint32_t h)
{
    uint8_t i,k;
    uint32_t j;
    short* ptr1 = NULL;
    short* ptr2 = NULL;
    short pixels = w*h;
    short kern_index = 0;
    uint32_t local_l2_maps = maps_info_ptr->l2_maps[DNUM];

    for (i = 0; i<local_l2_maps; i++)
    {
    	if(DNUM<maps_info_ptr->mod_l2)
    		kern_index = (DNUM*local_l2_maps)+i;
    	else
    		kern_index = (DNUM*local_l2_maps)+i+maps_info_ptr->mod_l2;

#ifndef FULLY_CONNECTED
    	uint8_t selection;
    	selection = rand() % L1_MAPS;
        ptr1 = (local_l1_maps_ptr+(selection*l1_size));
        Conv5x5(ptr1,temp2_ptr, w, h, &kernel5x5[kern_index][0]);
        for(k=1;k<L1_L2_CONNECTIONS;k=k+2)
        {
        	selection = rand() % L1_MAPS;
        	ptr2 = (local_l1_maps_ptr+(selection*l1_size));
            Conv5x5(ptr2,temp1_ptr, w, h, &kernel5x5[kern_index][0]);
        	DSP_add16_shift(temp2_ptr,temp1_ptr,layer1_ptr,pixels,1);
        	selection = rand() % L1_MAPS;
        	ptr2 = (local_l1_maps_ptr+(selection*l1_size));
        	Conv5x5(ptr2,temp1_ptr, w, h, &kernel5x5[kern_index][0]);
        	DSP_add16_shift(temp1_ptr,layer1_ptr,temp2_ptr,pixels,1);
        }
#else
    	ptr1 = local_l1_maps_ptr;
        Conv5x5(ptr1,temp2_ptr, w, h, &kernel5x5[kern_index][0]);
        for(k=1;k<L1_MAPS;k=k+2)
        {
        	ptr2 = local_l1_maps_ptr + (k*l1_size);
        	Conv5x5(ptr2,temp1_ptr, w, h, &kernel5x5[kern_index][0]);
        	DSP_add16_shift(temp2_ptr,temp1_ptr,layer1_ptr,pixels,1);
        	ptr2 = ptr2 + l1_size;
        	Conv5x5(ptr2,temp1_ptr, w, h, &kernel5x5[kern_index][0]);
        	DSP_add16_shift(temp1_ptr,layer1_ptr,temp2_ptr,pixels,1);
        }

#endif
        for (j=0; j<pixels; j++)
        {
        	temp2_ptr[j] = ((temp2_ptr[j] < 0) ? 0 : temp2_ptr[j]);
        }

        maxpool(temp2_ptr,(local_l2_maps_ptr+(i*l2_size)), w, h);
    }
}

void deeplearn(short* data, uint32_t w, uint32_t h)
{
	core_id = DNUM;

	MemMgr_HeapInit(w,h);

	if(DNUM==MASTER_CORE_ID)
	{
		INIT_DONE = 0x0;
	}

    operateLayer1(data, w, h);
    CACHE_wbAllL1dWait();

    /* All cores update the counter informing that they finished their iteration */
    while ((CSL_semAcquireDirect (LAYER_1_SEM)) == 0);

    L1_SYNC = L1_SYNC+1;

    /* Release the hardware semaphore. */
    CSL_semReleaseSemaphore (LAYER_1_SEM);

    /* All cores wait here to sync up */
    while (L1_SYNC != NUM_CORES);

    operateLayer2(w / 2, h / 2);
    CACHE_wbAllL1dWait();

    /* All cores update the counter informing that they finished their iteration */
    while ((CSL_semAcquireDirect (LAYER_2_SEM)) == 0);

    L2_SYNC = L2_SYNC+1;

    /* Release the hardware semaphore. */
    CSL_semReleaseSemaphore (LAYER_2_SEM);

    /* All cores wait here to sync up */
    while (L2_SYNC != NUM_CORES);
}
