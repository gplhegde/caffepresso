/*
 * mem_manager.c
 *
 *  Created on: 18 Nov 2016
 *      Author: Gopalakrishna Hegde
 */

#include "mem_manager.h"
#include <stdint.h>
#include <ti/csl/csl_cache.h>
#include "debug_control.h"



static char *p_shared_cur_free = (char *)MSMC_SHARED_SRAM_BASE;
void * shared_malloc(size_t size) {
	uint32_t no_blocks;
	char *p_new_free, *ptr;

	no_blocks = (size + MSMC_ALIGNMENT - 1) / MSMC_ALIGNMENT;
	p_new_free = p_shared_cur_free + no_blocks * MSMC_ALIGNMENT;

	if (p_new_free > (char *)MSMC_SHARED_SRAM_END) {
		REL_INFO("Cannot allocate memory on MSMC RAM\n");
		return NULL;
	}
	ptr = p_shared_cur_free;
	p_shared_cur_free = p_new_free;
	return ptr;
}

static char *p_private_cur_free = (char *)L2_PRIVATE_SRAM_BASE;
void * private_malloc(size_t size) {
	uint32_t no_blocks;
	char *p_new_free, *ptr;

	no_blocks = (size + L2_SRAM_ALIGNMENT - 1) / L2_SRAM_ALIGNMENT;
	p_new_free = p_private_cur_free + no_blocks * L2_SRAM_ALIGNMENT;

	if (p_new_free > (char *)L2_PRIVATE_SRAM_END) {
		REL_INFO("Cannot allocate memory on L2 SRAM\n");
		return NULL;
	}
	ptr = p_private_cur_free;
	p_private_cur_free = p_new_free;
	return ptr;
}


static char *p_ext_cur_free = (char *)DDR_SHARED_DRAM_BASE;
void * ext_malloc(size_t size) {
	uint32_t no_blocks;
	char *p_new_free, *ptr;



	no_blocks = (size + DRAM_ALIGNMENT - 1) / DRAM_ALIGNMENT;
	p_new_free = p_ext_cur_free + no_blocks * DRAM_ALIGNMENT;

	if (p_new_free > (char *)DDR_SHARED_DRAM_END) {
		REL_INFO("Cannot allocate memory DDR\n");
		return NULL;
	}
	ptr = p_ext_cur_free;
	p_ext_cur_free = p_new_free;
	return ptr;
}

void reset_mem_manager() {
	p_shared_cur_free = (char *)MSMC_SHARED_SRAM_BASE;
	p_ext_cur_free = (char *)DDR_SHARED_DRAM_BASE;
	p_private_cur_free = (char *)L2_PRIVATE_SRAM_BASE;
}
void shared_free(void *ptr) {
	// TODO
}
void private_free(void *ptr) {
	// TODO
}
void ext_free(void *ptr) {
	// TODO
}
