/*
 * data_sync.h
 *
 *  Created on: 4 Dec 2016
 *      Author: Gopalakrishna Hegde
 */

#ifndef _DATA_SYNC_H_
#define _DATA_SYNC_H_
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <ti/csl/csl_cacheAux.h>
#include <ti/csl/csl_semAux.h>
#include "user_config.h"
#include "caffe_proto_params.h"
// This must be multiple of cache line size
#define DATA_SYNC_OBJ_SIZE	(3*L1_CACHE_LINE_SIZE)

/* Macros for invalidating and writing back
 * Use these only when L2 cache is configured as SRAM
 *
 * addr : should be aligned to L1 cache line size
 * size : should be multiple for L1 cache line size
 * flag : to specify whether the call is blocking or non-blocking. Valid flags are from CACHE_Wait enum. Refer csl_cache.h
 */
#define L1_CACHE_INV(addr, size, flag)		CACHE_invL1d (addr, size, flag)

#define L1_CACHE_WB(addr, size, flag)		CACHE_wbL1d (addr, size, flag)

#ifdef USE_L2_CACHE
#define L2_CACHE_WB(addr, size, flag)		CACHE_wbL2(addr, size, flag)

#define L2_CACHE_INV(addr, size, flag)		CACHE_invL2 (addr, size, flag)
#else
#define L2_CACHE_WB(addr, size, flag)

#define L2_CACHE_INV(addr, size, flag)
#endif // USE_L2_CACHE
typedef struct {
	// flag to indicate global configuration status. Set by master core, read by all others
	uint32_t global_cfg_done;

	// flag to indicate local configuration status. Set and read by all cores
	uint32_t local_cfg_done;

	// flags used by the master core to indicate the init of image
	uint32_t img_init_done[2];

	// counting semaphores to perform data sync btw layers
	uint32_t sem_lyr_sync[NO_DEEP_LAYERS];

	/*
	 * Add more flags here
	 */

}SHARED_SYNC_OBJ_T;

typedef enum {
	GLOBAL_CFG_DONE = 0,
	LOCAL_CFG_DONE,
	IMAGE_INIT_DONE,
	SEM_LYR_SYNC,
	/*
	 * Add more flags here
	 */
	INVALID_MEMBER
} SYNC_MEMBER_E;

extern SHARED_SYNC_OBJ_T *p_sync_obj;

void get_global_sync_obj();

void wait_global_config();

void flag_global_config_done();

void flag_local_config_done();

void wait_all_local_config();

void signal_lyr_completion(uint32_t nn_lyr);

void wait_for_maps(uint32_t nn_lyr);

void reset_layer_sync_cntr();

void wait_for_image_init(uint32_t img_cnt);

void toggle_image_init_flag(uint32_t img_cnt);

#endif /* _DATA_SYNC_H_ */
