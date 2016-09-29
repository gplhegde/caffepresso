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

#pragma DATA_SECTION(local_l3_maps_ptr,".local_ram")
static short* local_l3_maps_ptr;

#pragma DATA_SECTION(pad_img_ptr,".local_ram")
static short* pad_img_ptr;

#pragma DATA_SECTION(pad_l2_ptr,".local_ram")
static short* pad_l2_ptr;

#pragma DATA_SECTION(pad_l3_ptr,".local_ram")
static short* pad_l3_ptr;

#pragma DATA_SECTION(temp_img_ptr,".local_ram")
static short* temp_img_ptr;

#pragma DATA_SECTION(layer1_ptr,".local_ram")
static short* layer1_ptr;

#pragma DATA_SECTION(layer2_ptr,".local_ram")
static short* layer2_ptr;

#pragma DATA_SECTION(accum1_ptr,".local_ram")
static short* accum1_ptr;

#pragma DATA_SECTION(accum2_ptr,".local_ram")
static short* accum2_ptr;

#pragma DATA_SECTION(l1_size,".local_ram")
static int l1_size;

#pragma DATA_SECTION(l2_size,".local_ram")
static int l2_size;

#pragma DATA_SECTION(l3_size,".local_ram")
static int l3_size;

typedef struct alloc_info {
char *free_ptr;
int block_count;
int max_blocks;
int block_size;
int max_size;
}alloc_info;

float *pInputNeurons, *pInputWt, *pHiddenBias, *pOutputWt;
float *pHiddenNeuron, *pOutputNeuron;

#ifdef FUNCTION_PROFILE
extern CSL_Uint64 startVal1,endVal1,startVal2,endVal2;
extern double layer1,layer2,layer3,conv1,conv2,conv3,pool1,pool2,pool3,rect1,rect2,rect3,add1,add2,pad1,pad2,pad3;
#endif
void dummy_classifier(float *pInputNeurons, int nInputNeurons, int nHiddenNeurons, int nOutputNeurons, float *pInputWt, float *pHiddenBias, float *pOutputWt);

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
	int l2_width,l2_height,l3_width,l3_height;
	alloc_info msmc_info,l2_info;
	int iN,oN,hN;

	l1_width 	= (img_width/L1_DOWN_SIZE_FACTOR)+(2*PAD_SIZE);
	l1_height 	= (img_height/L1_DOWN_SIZE_FACTOR)+(2*PAD_SIZE);
	l2_width 	= (l1_width/L2_DOWN_SIZE_FACTOR)+(2*PAD_SIZE);
	l2_height 	= (l1_height/L2_DOWN_SIZE_FACTOR)+(2*PAD_SIZE);
	l3_width 	= (l2_width/L3_DOWN_SIZE_FACTOR)+(2*PAD_SIZE);
	l3_height 	= (l2_height/L3_DOWN_SIZE_FACTOR)+(2*PAD_SIZE);

	l1_size = l1_width*l1_height;
	l2_size = l2_width*l2_height;
	l3_size = l3_width*l3_height;

	if(DNUM==MASTER_CORE_ID)
	{
		msmc_info.free_ptr = (char*)MSMC_REG_BASE;
		msmc_info.block_count = 0;
		msmc_info.block_size  = CACHE_L2_LINESIZE;
		msmc_info.max_size    = MSMC_SRAM_SIZE;
		msmc_info.max_blocks  = MSMC_SRAM_SIZE/CACHE_L2_LINESIZE;

		for(num_core=0; num_core<NUM_CORES;num_core++)
		{
			maps_info_ptr->l1_maps_ptr[num_core] = mem_alloc(&msmc_info,l1_width*l1_height*maps_info_ptr->l1_maps[num_core]*SHORT_SIZE); ///*TODO*/data allignment
			while(maps_info_ptr->l1_maps_ptr[num_core]==NULL);
		}
		for(num_core=0; num_core<NUM_CORES;num_core++)
		{

			maps_info_ptr->l2_maps_ptr[num_core] = mem_alloc(&msmc_info,l2_width*l2_height*maps_info_ptr->l2_maps[num_core]*SHORT_SIZE);
			while(maps_info_ptr->l2_maps_ptr[num_core]==NULL);
		}
		for(num_core=0; num_core<NUM_CORES;num_core++)
		{
			maps_info_ptr->l3_maps_ptr[num_core] = mem_alloc(&msmc_info,l3_width*l3_height*maps_info_ptr->l3_maps[num_core]*SHORT_SIZE);
			while(maps_info_ptr->l3_maps_ptr[num_core]==NULL);
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

	    while ((CSL_semAcquireDirect (LAYER_3_SEM)) == 0);

	    /* The core has completed local initialization */
	    maps_info_ptr->layer3_sync = 0;

	    /* Release the hardware semaphore. */
	    CSL_semReleaseSemaphore ( LAYER_3_SEM);

		CSL_semReleaseSemaphore(INIT_DONE_SEM);
	}

	while(!CSL_semIsFree(INIT_DONE_SEM));

	l2_info.free_ptr = (char*)L2_HEAP_BASE;
	l2_info.block_count = 0;
	l2_info.block_size  = CACHE_L1D_LINESIZE;
	l2_info.max_size    = L2_HEAP_SIZE;
	l2_info.max_blocks  = L2_HEAP_SIZE/CACHE_L1D_LINESIZE;

	pad_img_ptr		= mem_alloc(&l2_info,(img_width+2*PAD_SIZE)*(img_height+2*PAD_SIZE)*SHORT_SIZE);
	while(pad_img_ptr==NULL);
	temp_img_ptr 	= mem_alloc(&l2_info,img_width*img_height*SHORT_SIZE);
	while(temp_img_ptr==NULL);
	accum1_ptr  	= mem_alloc(&l2_info,l1_width*l1_height*SHORT_SIZE);
	while(accum1_ptr==NULL);


	layer1_ptr  	= mem_alloc(&l2_info,l1_width*l1_height*SHORT_SIZE);
	while(layer1_ptr==NULL);
	pad_l2_ptr 		= mem_alloc(&l2_info,(l1_width+2*PAD_SIZE)*(l1_height+2*PAD_SIZE)*SHORT_SIZE);
	while(pad_l2_ptr==NULL);
	accum2_ptr  	= mem_alloc(&l2_info,l2_width*l2_height*SHORT_SIZE);
	while(accum2_ptr==NULL);


	layer2_ptr  	= mem_alloc(&l2_info,l2_width*l2_height*SHORT_SIZE);
	while(layer2_ptr==NULL);
	pad_l3_ptr 		= mem_alloc(&l2_info,(l2_width+2*PAD_SIZE)*(l2_height+2*PAD_SIZE)*SHORT_SIZE);
	while(pad_l3_ptr==NULL);

	if(DNUM == MASTER_CORE_ID)
	{
		iN = 64 * ((12 - 5 + 1)/2) * ((12 - 5 + 1)/2);
		//iN = 50*9;
		hN = 64;
		oN = 10;

		pInputNeurons 	= mem_alloc(&l2_info,iN*sizeof(float));
		while(pInputNeurons==NULL);
		pInputWt 		= mem_alloc(&l2_info,hN*iN*sizeof(float));
		while(pInputWt==NULL);
		pHiddenBias 	= mem_alloc(&l2_info,hN*sizeof(float));
		while(pHiddenBias==NULL);
		pOutputWt 		= mem_alloc(&l2_info,hN*oN*sizeof(float));
		while(pOutputWt==NULL);
	}

	local_l1_maps_ptr= (short*)maps_info_ptr->l1_maps_ptr[DNUM];
	local_l2_maps_ptr= (short*)maps_info_ptr->l2_maps_ptr[DNUM];
	local_l3_maps_ptr= (short*)maps_info_ptr->l3_maps_ptr[DNUM];
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

Conv5x5(short* src_ptr,short* out_ptr,uint32_t w,uint32_t h,const short*mask_ptr)
{
    short shift = 0; /*TODO*/
    int col;

    for(col=0;col<h-4;col++)
    {
    	IMG_conv_5x5_i16s_c16s ((src_ptr+col*w), (out_ptr+col*w), w, w+2*PAD_SIZE, mask_ptr,shift);
    }
}

do_padding(short*image,short*dest_image, uint32_t M, uint32_t N, uint8_t col_pad_width, uint8_t row_pad_width)
{
	int row, col;
	short* temp = dest_image;
	temp = temp + (col_pad_width+N+col_pad_width)*row_pad_width;
    for (row = 0; row < M; row=row++)
    {
    	temp = temp + col_pad_width;
        for (col = 0; col < N; col=col++)
        {
        	*(temp+N*row+col) = *(image + N*row + col);
        }
        temp = temp + col_pad_width;
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
#ifdef FUNCTION_PROFILE
	startVal2 = _itoll(TSCH,TSCL);
#endif
    	do_padding(img_ptr,pad_img_ptr,w,h,PAD_SIZE,PAD_SIZE);
#ifdef FUNCTION_PROFILE
	endVal2 = _itoll(TSCH,TSCL);
	pad1 += ((endVal2-startVal2)/DSP_FREQ_IN_MHZ);
#endif
#ifdef FUNCTION_PROFILE
	startVal2 = _itoll(TSCH,TSCL);
#endif
	if(DNUM<maps_info_ptr->mod_l1)
		kern_index = (DNUM*local_l1_maps)+i;
	else
		kern_index = (DNUM*local_l1_maps)+i+maps_info_ptr->mod_l1;

    	Conv5x5(pad_img_ptr,temp_img_ptr,w,h,&kernel5x5[kern_index][0]);
#ifdef FUNCTION_PROFILE
	endVal2 = _itoll(TSCH,TSCL);
	conv1 += ((endVal2-startVal2)/DSP_FREQ_IN_MHZ);
#endif
#ifdef FUNCTION_PROFILE
	startVal2 = _itoll(TSCH,TSCL);
#endif
    	for (j=0; j<pixels; j++)
        {
    		temp_img_ptr[j] = ((temp_img_ptr[j] < 0) ? 0 : temp_img_ptr[j]);
        }
#ifdef FUNCTION_PROFILE
	endVal2 = _itoll(TSCH,TSCL);
	rect1 += ((endVal2-startVal2)/DSP_FREQ_IN_MHZ);
#endif
#ifdef FUNCTION_PROFILE
	startVal2 = _itoll(TSCH,TSCL);
#endif
        maxpool(temp_img_ptr,local_l1_maps_ptr+(i*l1_size), w, h);
#ifdef FUNCTION_PROFILE
	endVal2 = _itoll(TSCH,TSCL);
	pool1 += ((endVal2-startVal2)/DSP_FREQ_IN_MHZ);
#endif
    }
}

operateLayer2(uint32_t w, uint32_t h)
{
    uint8_t i,k;
    uint32_t j;
    short pixels = w*h;
    short kern_index = 0;
    uint32_t local_l2_maps = maps_info_ptr->l2_maps[DNUM];

    for (i = 0; i<local_l2_maps; i++)
    {
#ifdef FUNCTION_PROFILE
	startVal2 = _itoll(TSCH,TSCL);
#endif
    	if(DNUM<maps_info_ptr->mod_l2)
    		kern_index = (DNUM*local_l2_maps)+i;
    	else
    		kern_index = (DNUM*local_l2_maps)+i+maps_info_ptr->mod_l2;
        for(i=0;i<L1_MAPS;i++)
        {
        	pad_l2_ptr = maps_info_ptr->l1_maps_ptr[0]+(i*l1_size);
            Conv5x5(pad_l2_ptr,accum1_ptr, w, h, &kernel5x5[kern_index][0]);
            DSP_add16_shift(layer1_ptr,accum1_ptr,layer1_ptr,(w*h/4),1);
        }
#ifdef FUNCTION_PROFILE
	endVal2 = _itoll(TSCH,TSCL);
	conv2 += ((endVal2-startVal2)/DSP_FREQ_IN_MHZ);
#endif
#ifdef FUNCTION_PROFILE
	startVal2 = _itoll(TSCH,TSCL);
#endif
        for (j=0; j<pixels; j++)
        {
        	layer1_ptr[j] = ((layer1_ptr[j] < 0) ? 0 : layer1_ptr[j]);
        }
#ifdef FUNCTION_PROFILE
	endVal2 = _itoll(TSCH,TSCL);
	rect2 += ((endVal2-startVal2));
#endif
#ifdef FUNCTION_PROFILE
	startVal2 = _itoll(TSCH,TSCL);
#endif
        maxpool(layer1_ptr,local_l2_maps_ptr+(i*l2_size), w, h);
#ifdef FUNCTION_PROFILE
	endVal2 = _itoll(TSCH,TSCL);
	pool2 += ((endVal2-startVal2)/DSP_FREQ_IN_MHZ);
#endif
    }
}

operateLayer3(uint32_t w, uint32_t h)
{
	uint8_t i,k;
	uint32_t j;
	short pixels = w*h;
	short kern_index = 0;
	uint32_t local_l3_maps = maps_info_ptr->l3_maps[DNUM];

	for (i = 0; i<local_l3_maps; i++)
	{
#ifdef FUNCTION_PROFILE
	startVal2 = _itoll(TSCH,TSCL);
#endif
		if(DNUM<maps_info_ptr->mod_l2)
			kern_index = (DNUM*local_l3_maps)+i;
		else
			kern_index = (DNUM*local_l3_maps)+i+maps_info_ptr->mod_l2;

		for(i=0;i<L2_MAPS;i++)
        {
        	pad_l3_ptr = maps_info_ptr->l2_maps_ptr[0]+(i*l2_size);
            Conv5x5(pad_l3_ptr,accum2_ptr, w, h, &kernel5x5[kern_index][0]);
            DSP_add16_shift(accum2_ptr,layer2_ptr,layer2_ptr,(w*h/4),1);
        }

#ifdef FUNCTION_PROFILE
	endVal2 = _itoll(TSCH,TSCL);
	conv3 += ((endVal2-startVal2)/DSP_FREQ_IN_MHZ);
#endif
#ifdef FUNCTION_PROFILE
	startVal2 = _itoll(TSCH,TSCL);
#endif
		for (j=0; j<pixels; j++)
		{
			layer2_ptr[j] = ((layer2_ptr[j] < 0) ? 0 : layer2_ptr[j]);
		}
#ifdef FUNCTION_PROFILE
	endVal2 = _itoll(TSCH,TSCL);
	rect3 += ((endVal2-startVal2));
#endif
#ifdef FUNCTION_PROFILE
	startVal2 = _itoll(TSCH,TSCL);
#endif
		maxpool(layer2_ptr,local_l3_maps_ptr+i*l3_size, w, h);
#ifdef FUNCTION_PROFILE
	endVal2 = _itoll(TSCH,TSCL);
	pool3 += ((endVal2-startVal2));
#endif
	}
}

void deeplearn(short* data, uint32_t w, uint32_t h)
{
	core_id = DNUM;

	MemMgr_HeapInit((w+2*PAD_SIZE),(h+2*PAD_SIZE));
#ifdef FUNCTION_PROFILE
	startVal1 = _itoll(TSCH,TSCL);
#endif
    operateLayer1(data, w, h);
#ifdef FUNCTION_PROFILE
	endVal1 = _itoll(TSCH,TSCL);
	layer1 += ((endVal1-startVal1)/DSP_FREQ_IN_MHZ);
#endif
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
#ifdef FUNCTION_PROFILE
	startVal1 = _itoll(TSCH,TSCL);
#endif
    operateLayer2(w / 2, h / 2);
#ifdef FUNCTION_PROFILE
	endVal1 = _itoll(TSCH,TSCL);
	layer2 += ((endVal1-startVal1)/DSP_FREQ_IN_MHZ);
#endif
    CACHE_wbAllL1dWait();

    /* All cores update the counter informing that they finished their iteration */
    while ((CSL_semAcquireDirect (LAYER_2_SEM)) == 0);

    maps_info_ptr->layer2_sync++;

    /* Release the hardware semaphore. */
    CSL_semReleaseSemaphore (LAYER_2_SEM);

    /* All cores wait here to sync up */
    while (maps_info_ptr->layer2_sync != NUM_CORES);
#ifdef FUNCTION_PROFILE
	startVal1 = _itoll(TSCH,TSCL);
#endif
    operateLayer3(w / 4, h / 4);
#ifdef FUNCTION_PROFILE
	endVal1 = _itoll(TSCH,TSCL);
	layer3 += ((endVal1-startVal1)/DSP_FREQ_IN_MHZ);
#endif
    CACHE_wbAllL1dWait();

//    /* All cores update the counter informing that they finished their iteration */
//    while ((CSL_semAcquireDirect (LAYER_3_SEM)) == 0);
//
//    maps_info_ptr->layer3_sync++;
//
//    /* Release the hardware semaphore. */
//    CSL_semReleaseSemaphore (LAYER_3_SEM);
//
//    /* All cores wait here to sync up */
//    while (maps_info_ptr->layer3_sync != NUM_CORES);

	// dummy classifier
	dummy_classifier(pInputNeurons,64 * ((12 - 5 + 1)/2) * ((12 - 5 + 1)/2) , HIDDEN_NEURONS, 10, pInputWt, pHiddenBias, pOutputWt);

}
