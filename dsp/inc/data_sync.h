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

// This must be multiple of cache line size
#define DATA_SYNC_OBJ_SIZE	(L1_CACHE_LINE_SIZE)

/* Macros for invalidating and writing back
 * Use these only when L2 cache is configured as SRAM
 *
 * addr : should be aligned to L1 cache line size
 * size : should be multiple for L1 cache line size
 * flag : to specify whether the call is blocking or non-blocking. Valid flags are from CACHE_Wait enum. Refer csl_cache.h
 */
#define L1_CACHE_INV(addr, size, flag)    CACHE_invL1d (addr, size, flag)

#define L1_CACHE_WB(addr, size, flag)     CACHE_wbL1d (addr, size, flag)


typedef struct {
	// flag to indicate global configuration status. Set by master core, read by all others
	uint32_t global_cfg_done;

	// flag to indicate local configuration status. Set and read by all cores
	uint32_t local_cfg_done;

	// flags used by the master core to indicate the init of image
	uint32_t img_init_done[2];

	// counting semaphores to perform data sync btw layers
	uint32_t sem_lyr_sync[2];

	/*
	 * Add more flags here
	 */

	uint8_t invalid_mem;
}SHARED_SYNC_OBJ_T;

typedef enum {
	GLOBAL_CFG_DONE = 0,
	LOCAL_CFG_DONE,
	IMAGE_INIT_DONE_0,
	IMAGE_INIT_DONE_1,
	SEM_LYR_SYNC_0,
	SEM_LYR_SYNC_1,
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

void reset_layer_sync_cntr(uint32_t nn_lyr);

void wait_for_image_init(uint32_t img_cnt);

void toggle_image_init_flag(uint32_t img_cnt);

#endif /* _DATA_SYNC_H_ */
