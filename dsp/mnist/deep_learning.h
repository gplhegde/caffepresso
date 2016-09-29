/*******************************************************
* File Name -- main.c
*
* Authors:  Nachiappan, Gourav, Mahesh
*
* Created on : 27-September-2015
*
********************************************************/

#ifndef DSP_DEEPLEARN_DEEP_LEARNING_H_
#define DSP_DEEPLEARN_DEEP_LEARNING_H_

#include "user_config.h"

typedef struct maps_info
{
	 uint8_t l1_maps[NUM_CORES];
	 uint8_t l2_maps[NUM_CORES];
	 uint8_t mod_l1;
	 uint8_t mod_l2;
	 short* l1_maps_ptr[NUM_CORES];
     short* l2_maps_ptr[NUM_CORES];
     volatile int layer1_sync;
     volatile int layer2_sync;
}maps_info;

maps_info* maps_info_ptr;

void deeplearn(short* data, uint32_t w, uint32_t h);


#endif /* DSP_DEEPLEARN_DEEP_LEARNING_H_ */
