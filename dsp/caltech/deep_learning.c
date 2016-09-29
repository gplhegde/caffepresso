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

#include "inc/kernel_11x11.h"

#include <ti/csl/csl_semAux.h>

//#define FULLY_CONNECTED

#define MIN_DSPMAX_ELEMENTS 100

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

	l1_width = img_width/L1_DOWN_SIZE_FACTOR;
	l1_height = img_height/L1_DOWN_SIZE_FACTOR;
	l2_width = l1_width/L2_DOWN_SIZE_FACTOR;
	l2_height = l1_height/L2_DOWN_SIZE_FACTOR;

	l1_size = l1_width*l1_height;
	l2_size = l2_width*l2_height;

	if(DNUM==MASTER_CORE_ID)
	{
		msmc_info.free_ptr = (char*)MSMC_REG_BASE;
		msmc_info.block_count = 0;
		msmc_info.block_size  = CACHE_L2_LINESIZE;
		msmc_info.max_size    = MSMC_SRAM_SIZE;
		msmc_info.max_blocks  = MSMC_SRAM_SIZE/CACHE_L2_LINESIZE;

		maps_info_ptr->img_ptr = mem_alloc(&msmc_info,img_width*img_height*sizeof(short));

		for(num_core=0; num_core<NUM_CORES;num_core++)
		{
#ifndef FULLY_CONNECTED
			maps_info_ptr->l1_maps_ptr[num_core] = mem_alloc(&msmc_info,l1_width*l1_height*maps_info_ptr->l1_maps[num_core]*sizeof(short)); ///*TODO*/data allignment
			while(maps_info_ptr->l1_maps_ptr[num_core]==NULL);
#else
			maps_info_ptr->l1_maps_ptr[num_core] = mem_alloc(&msmc_info,l1_width*l1_height*sizeof(short)); ///*TODO*/data allignment
			while(maps_info_ptr->l1_maps_ptr[num_core]==NULL);
#endif
		}
		for(num_core=0; num_core<NUM_CORES;num_core++)
		{
			maps_info_ptr->l2_maps_ptr[num_core] = mem_alloc(&msmc_info,l2_width*l2_height*maps_info_ptr->l2_maps[num_core]*sizeof(short));
			while(maps_info_ptr->l2_maps_ptr[num_core]==NULL);
		}
	    while ((CSL_semAcquireDirect (LAYER_1_SEM)) == 0);

	    /* The core has completed local initialization */
	    maps_info_ptr->layer1_sync = 0;

	    /* Release the hardware semaphore. */
	    CSL_semReleaseSemaphore ( LAYER_1_SEM);

	    while ((CSL_semAcquireDirect (LAYER_2_SEM)) == 0);

	    /* The core has completed local initialization */
	    maps_info_ptr->layer2_sync = 0;

	    /* Release the hardware semaphore. */
	    CSL_semReleaseSemaphore ( LAYER_2_SEM);

	    CSL_semReleaseSemaphore(INIT_DONE_SEM);
	}

	while(!CSL_semIsFree(INIT_DONE_SEM));

	l2_info.free_ptr = (char*)L2_HEAP_BASE;
	l2_info.block_count = 0;
	l2_info.block_size  = CACHE_L1D_LINESIZE;
	l2_info.max_size    = L2_HEAP_SIZE;
	l2_info.max_blocks  = L2_HEAP_SIZE/CACHE_L1D_LINESIZE;

	memset((void*)L2_HEAP_BASE,0x0,L2_HEAP_SIZE);

	temp_img_ptr 	= mem_alloc(&l2_info,img_width*img_height*sizeof(short));
	while(temp_img_ptr==NULL);
	layer1_ptr  	= mem_alloc(&l2_info,l1_width*l1_height*sizeof(short));
	while(layer1_ptr==NULL);
	temp1_ptr  		= mem_alloc(&l2_info,l1_width*l1_height*sizeof(short));
	while(temp1_ptr==NULL);

	local_l1_maps_ptr= (short*)maps_info_ptr->l1_maps_ptr[DNUM];
	local_l2_maps_ptr= (short*)maps_info_ptr->l2_maps_ptr[DNUM];
}

static void maxpool_l1(short *image,short *dest_image, int M, int N)
{
	int krows =L1_AVG_POOL,kcols =L1_AVG_POOL,stride_len=L1_DOWN_SIZE_FACTOR;
    int row, col,kernel_row, kernel_col;
    int dest_col = N/L1_DOWN_SIZE_FACTOR,drows=0,dcols=0;

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
        	*(dest_image + dest_col*drows + dcols) = DSP_maxval(&dilate[0],L1_AVG_POOL*L1_AVG_POOL);
        }
    }

}

static void maxpool_l2(short *image,short *dest_image, int M, int N)
{
	int krows =L2_AVG_POOL,kcols =L2_AVG_POOL,stride_len=L2_DOWN_SIZE_FACTOR;
    int row, col,kernel_row, kernel_col;
    int dest_col = N/L2_DOWN_SIZE_FACTOR,drows=0,dcols=0;

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
        	*(dest_image + dest_col*drows + dcols) = DSP_maxval(&dilate[0],L2_AVG_POOL*L2_AVG_POOL);
        }
    }

}
Conv9x9(short* src_ptr,short* out_ptr,uint32_t w,uint32_t h,const short*mask_ptr)
{
    short shift = 0;
    int col;

    for(col=0;col<h-8;col++)
    {
    	IMG_conv_11x11_i16s_c16s ((src_ptr+col*w), (out_ptr+col*w), w, w, mask_ptr,shift);
    	//IMG_conv_9x9_i16s_c16s_cn ((src_ptr+col*w), (out_ptr+col*w), w, w, mask_ptr,shift);
    }
}
operateLayer1(uint32_t w, uint32_t h)
{
    uint32_t i;
    uint32_t j;
    unsigned short pixels = w*h;
    short kern_index;
    uint32_t local_l1_maps = maps_info_ptr->l1_maps[DNUM];

    for (i = 0; i<local_l1_maps; i++)
    {
    	if(DNUM<maps_info_ptr->mod_l1)
    		kern_index = (DNUM*local_l1_maps)+i;
    	else
    		kern_index = (DNUM*local_l1_maps)+i+maps_info_ptr->mod_l1;

    	Conv9x9(maps_info_ptr->img_ptr,temp_img_ptr,w,h,&kernel11x11[kern_index][0]);

    	for (j=0; j<pixels; j++)
        {
    		temp_img_ptr[j] = ((temp_img_ptr[j] < 0) ? 0 : temp_img_ptr[j]);
        }
#ifndef FULLY_CONNECTED
    	maxpool_l1(temp_img_ptr,(local_l1_maps_ptr+(i*l1_size)), w, h);
#else
    	maxpool_l1(temp_img_ptr,temp1_ptr, w, h);
    	DSP_add16_shift(local_l1_maps_ptr,temp1_ptr,local_l1_maps_ptr,(w*h/4),1);
#endif
    }
}

operateLayer2(uint32_t w, uint32_t h)
{
    uint8_t i,k;
    uint32_t j;
    short* ptr1 = NULL;
    unsigned short pixels = w*h;
    short kern_index = 0;
    uint32_t local_l2_maps = maps_info_ptr->l2_maps[DNUM];

#ifdef FULLY_CONNECTED
    ptr1 = layer1_ptr;
    for(k=0;k<NUM_CORES;k++)
    	DSP_add16_shift((short*)maps_info_ptr->l1_maps_ptr[k],ptr1,ptr1,pixels,1);
#endif
    for (i = 0; i<local_l2_maps; i++)
    {
    	if(DNUM<maps_info_ptr->mod_l2)
    		kern_index = (DNUM*local_l2_maps)+i;
    	else
    		kern_index = (DNUM*local_l2_maps)+i+maps_info_ptr->mod_l2;

#ifndef FULLY_CONNECTED
    	uint8_t selection;
        short* global_l1_map_start_ptr = (short*)maps_info_ptr->l1_maps_ptr[0];
        short* ptr2 = NULL;
//
//    	selection = rand() % L1_MAPS;
//        ptr1 = (global_l1_map_start_ptr+(selection*l1_size));
//        for(k=0;k<L1_L2_CONNECTIONS/2;k++)
//        {
//        	selection = rand() % L1_MAPS;
//        	ptr2 = (global_l1_map_start_ptr+(selection*l1_size));
//        	DSP_add16_shift(ptr1,ptr2,temp1_ptr,pixels,1);
//        	selection = rand() % L1_MAPS;
//        	ptr2 = (global_l1_map_start_ptr+(selection*l1_size));
//        	DSP_add16_shift(temp1_ptr,ptr2,ptr1,pixels,1);
//        }
#endif
        for(k=0;k<L1_MAPS;k++)
        {
        	ptr1= (global_l1_map_start_ptr+(k*l1_size));
            Conv9x9(ptr1,temp1_ptr, w, h, &kernel11x11[kern_index][0]);
        	DSP_add16_shift(temp1_ptr,layer1_ptr,layer1_ptr,pixels,1);
        }

        for (j=0; j<pixels; j++)
        {
        	layer1_ptr[j] = ((layer1_ptr[j] < 0) ? 0 : layer1_ptr[j]);
        }

        maxpool_l2(layer1_ptr,(local_l2_maps_ptr+(i*l2_size)), w, h);
    }
}
//void MemMgr_HeapDeInit()
//{
//	int num_core;
//
//	if(DNUM == MASTER_CORE_ID)
//	{
//		free(maps_info_ptr->img_ptr);
//		for(num_core=0; num_core<NUM_CORES;num_core++)
//		{
//			free(maps_info_ptr->l1_maps_ptr[num_core]);
//			free(maps_info_ptr->l2_maps_ptr[num_core]);
//		}
//	}
//	free(temp_img_ptr);
//	free(layer1_ptr);
//	free(temp1_ptr);
//}
void deeplearn( uint32_t w, uint32_t h)
{
	core_id = DNUM;

	MemMgr_HeapInit(w,h);

    operateLayer1(w, h);
    CACHE_wbAllL1dWait();

    /* All cores update the counter informing that they finished their iteration */
    while ((CSL_semAcquireDirect (LAYER_1_SEM)) == 0);

    maps_info_ptr->layer1_sync++;

    /* Release the hardware semaphore. */
    CSL_semReleaseSemaphore (LAYER_1_SEM);

    /* All cores wait here to sync up */
    while (maps_info_ptr->layer1_sync != NUM_CORES);
	if(DNUM==MASTER_CORE_ID)
		CSL_semAcquireDirect(INIT_DONE_SEM);
    operateLayer2(w / L1_DOWN_SIZE_FACTOR, h / L1_DOWN_SIZE_FACTOR);
    CACHE_wbAllL1dWait();

//    /* All cores update the counter informing that they finished their iteration */
//    while ((CSL_semAcquireDirect (LAYER_2_SEM)) == 0);
//
//    maps_info_ptr->layer2_sync++;
//
//    /* Release the hardware semaphore. */
//    CSL_semReleaseSemaphore (LAYER_2_SEM);
//
//    /* All cores wait here to sync up */
//    while (maps_info_ptr->layer2_sync != 0x8);

//	MemMgr_HeapDeInit();
}
