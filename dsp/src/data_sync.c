/*
 * data_sync.c
 *
 *  Created on: 4 Dec 2016
 *      Author: Gopalakrishna Hegde
 */
#include "data_sync.h"
#include "debug_control.h"
#include "mem_manager.h"

#if DATA_SYNC_OBJ_SIZE > L1_CACHE_LINE_SIZE
#warning "Consider invalidating only necessary cache lines in shared sync object in get_sync_member() function"
#endif

extern unsigned int core_id;

#pragma DATA_ALIGN   (shared_sync_mem, L1_CACHE_LINE_SIZE)
#pragma DATA_SECTION (shared_sync_mem, ".shared_ocm")
uint8_t shared_sync_mem[DATA_SYNC_OBJ_SIZE];

// local pointer to shared sync structure object
SHARED_SYNC_OBJ_T *p_sync_obj;


static uint32_t get_sync_member(SYNC_MEMBER_E member, uint32_t index) {
	uint32_t member_val;

	// TODO: Instead of invalidating entire sync object, invalidate only necessary cache line
	// This needs to be considered when the sync object size exceeds the cache line size
	L1_CACHE_INV((void *)p_sync_obj, DATA_SYNC_OBJ_SIZE, CACHE_WAIT);
	L2_CACHE_INV((void *)p_sync_obj, DATA_SYNC_OBJ_SIZE, CACHE_WAIT);
	switch(member) {
		case GLOBAL_CFG_DONE:
			member_val = (uint32_t)p_sync_obj->global_cfg_done;
			break;
		case LOCAL_CFG_DONE:
			member_val = (uint32_t)p_sync_obj->local_cfg_done;
			break;
		case IMAGE_INIT_DONE:
			member_val = (uint32_t)p_sync_obj->img_init_done[index];
			break;
		case SEM_LYR_SYNC:
			member_val = (uint32_t)p_sync_obj->sem_lyr_sync[index];
			break;
		/*
		 * Add get value for extra members here
		 */
		case INVALID_MEMBER:
		default:
			REL_INFO("Invalid global shared sync member\n");
	}
	return member_val;
}

/*
 * Get the pointer to the global sync object. Note that the pointer is local
 */
void get_global_sync_obj() {
	p_sync_obj = (SHARED_SYNC_OBJ_T *)shared_sync_mem;
}

/* Reset the flags and counters in the global sync object and flag the completion of global init
 * Only to be called from the master core!
 */
void flag_global_config_done() {

	REL_ASSERT(core_id == MASTER_CORE_ID);

	// reset the global sync object to zero
	memset((void *)shared_sync_mem, 0, sizeof(shared_sync_mem));
	/*p_sync_obj->local_cfg_done = 0;
	p_sync_obj->sem_lyr_sync[0] = 0;
	p_sync_obj->sem_lyr_sync[1] = 0;
	p_sync_obj->img_init_done[0] = FALSE;
	p_sync_obj->img_init_done[1] = FALSE;*/
	/*
	 * Add reset for future flags here
	 */

	// finally set the config done flag and flush the cache
	p_sync_obj->global_cfg_done = TRUE;
	// write back is not required if the shared data struct is placed on memory segment for which
	// write-through mode is set in L1D cache controller
	L1_CACHE_WB((void *)p_sync_obj, sizeof(shared_sync_mem), CACHE_WAIT);
	L2_CACHE_WB((void *)p_sync_obj, sizeof(shared_sync_mem), CACHE_WAIT);
}

void wait_global_config() {
	uint32_t global_cfg_done;

	do {
		global_cfg_done = (uint32_t) get_sync_member(GLOBAL_CFG_DONE, 0);
	} while(!global_cfg_done);
}

void flag_local_config_done() {

	// take the lock
	while ((CSL_semAcquireDirect (SHARED_MEM_SEM)) == 0);

	// increment the local configuration done counter
	p_sync_obj->local_cfg_done++;

	// write back the updated counter
	// write back is not required if the shared data struct is placed on memory segment for which
	// write-through mode is set in L1D cache controller
	L1_CACHE_WB((void *)p_sync_obj, DATA_SYNC_OBJ_SIZE, CACHE_WAIT);
	L2_CACHE_WB((void *)p_sync_obj, DATA_SYNC_OBJ_SIZE, CACHE_WAIT);

	// release the lock
	CSL_semReleaseSemaphore (SHARED_MEM_SEM);
}

void wait_all_local_config() {
	uint32_t local_cfg_done;

	do {
		local_cfg_done = (uint32_t) get_sync_member(LOCAL_CFG_DONE, 0);
	} while(local_cfg_done != NO_CORES);
}

void signal_lyr_completion(uint32_t nn_lyr) {
	//SYNC_MEMBER_E member;

	//member = (nn_lyr % 2) == 0 ? SEM_LYR_SYNC_0: SEM_LYR_SYNC_1;
	// take the lock
	while ((CSL_semAcquireDirect (SHARED_MEM_SEM)) == 0);

	// FIXME: Need to invalidate L1D before reading and incrementing?
	p_sync_obj->sem_lyr_sync[nn_lyr % NO_DEEP_LAYERS] = get_sync_member(SEM_LYR_SYNC, nn_lyr % NO_DEEP_LAYERS) + 1;


	// write back the updated counter
	// write back is not required if the shared data struct is placed on memory segment for which
	// write-through mode is set in L1D cache controller
	L1_CACHE_WB((void *)p_sync_obj, DATA_SYNC_OBJ_SIZE, CACHE_WAIT);
	L2_CACHE_WB((void *)p_sync_obj, DATA_SYNC_OBJ_SIZE, CACHE_WAIT);

	// release the lock
	CSL_semReleaseSemaphore (SHARED_MEM_SEM);
}

void wait_for_maps(uint32_t nn_lyr) {
	uint32_t sem_cnt;
	//SYNC_MEMBER_E member;

	//member = (nn_lyr % 2) == 0 ? SEM_LYR_SYNC_0 : SEM_LYR_SYNC_1;
	do {
		sem_cnt = (uint32_t) get_sync_member(SEM_LYR_SYNC, nn_lyr % NO_DEEP_LAYERS);
	} while(sem_cnt != NO_CORES);

	// invalidate all cached data from the map buffers in MSMC RAM except the shared data structures in the first 1KB region.
	// this is because each core needs the output of all cores for the next layer.
	L1_CACHE_INV((void *)MSMC_SHARED_SRAM_BASE, MSMC_SHARED_SRAM_END - MSMC_SHARED_SRAM_BASE + 1, CACHE_WAIT);
	L2_CACHE_INV((void *)MSMC_SHARED_SRAM_BASE, MSMC_SHARED_SRAM_END - MSMC_SHARED_SRAM_BASE + 1, CACHE_WAIT);

}

// This must be only called from the master core
void reset_layer_sync_cntr() {
	// take the lock
	// TODO: lock is not required since only master core resets these counters
	while ((CSL_semAcquireDirect (SHARED_MEM_SEM)) == 0);

	// need to reset all counters btw two successive image inferences
	memset((void *)p_sync_obj->sem_lyr_sync, 0, sizeof(p_sync_obj->sem_lyr_sync));
	//p_sync_obj->sem_lyr_sync[nn_lyr] = 0;

	// write back the updated counter
	// write back is not required if the shared data struct is placed on memory segment for which
	// write-through mode is set in L1D cache controller
	L1_CACHE_WB((void *)p_sync_obj, DATA_SYNC_OBJ_SIZE, CACHE_WAIT);
	L2_CACHE_WB((void *)p_sync_obj, DATA_SYNC_OBJ_SIZE, CACHE_WAIT);
	// release the lock
	CSL_semReleaseSemaphore (SHARED_MEM_SEM);
}

void wait_for_image_init(uint32_t img_cnt) {
	uint32_t flag;

	do {
		flag = (uint32_t) get_sync_member(IMAGE_INIT_DONE, (img_cnt % 2));
	} while(!flag);
	// Since master core has just updated the image buffer, invalidate cache.
	L1_CACHE_INV((void *)MSMC_SHARED_SRAM_BASE, MSMC_SHARED_SRAM_END - MSMC_SHARED_SRAM_BASE + 1, CACHE_WAIT);
	L2_CACHE_INV((void *)MSMC_SHARED_SRAM_BASE, MSMC_SHARED_SRAM_END - MSMC_SHARED_SRAM_BASE + 1, CACHE_WAIT);
}

// This must be only called from the master core
void toggle_image_init_flag(uint32_t img_cnt) {
	// take the lock
	// TODO: lock is not required since only master core resets these counters
	while ((CSL_semAcquireDirect (SHARED_MEM_SEM)) == 0);

	// need to reset the other semaphore that was used for previous layer
	if(p_sync_obj->img_init_done[img_cnt % 2]) {
		p_sync_obj->img_init_done[img_cnt % 2] = FALSE;
	} else {
		p_sync_obj->img_init_done[img_cnt % 2] = TRUE;
	}

	// write back the updated counter
	// write back is not required if the shared data struct is placed on memory segment for which
	// write-through mode is set in L1D cache controller
	L1_CACHE_WB((void *)p_sync_obj, DATA_SYNC_OBJ_SIZE, CACHE_WAIT);
	L2_CACHE_WB((void *)p_sync_obj, DATA_SYNC_OBJ_SIZE, CACHE_WAIT);
	// release the lock
	CSL_semReleaseSemaphore (SHARED_MEM_SEM);
}

void _mfence() {
	// Stub to avoid linker error
}

