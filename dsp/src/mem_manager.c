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

#define L2_SRAM_ALIGNMENT CACHE_L1D_LINESIZE
#define MSMC_ALIGNMENT CACHE_L1D_LINESIZE
#define DRAM_ALIGNMENT CACHE_L1D_LINESIZE

#ifdef DEVICE_K2H

#define DSP_FREQ_IN_MHZ			1400
#define L2_SRAM_SIZE			(0x00100000)
#define MSMC_SRAM_SIZE 			(6*1024*1024)
#define DDR_RAM_SIZE 			(0x80000000)

#else
#define DSP_FREQ_IN_MHZ			1000
#define MSMC_SRAM_SIZE 			(4*1024*1024)
#endif


// FIXME: base should be move to the end of .local_ram segment in linker script. Currently an offset of 0x30000 is used looking at map file.
#define L2_PRIVATE_SRAM_BASE 	(CSL_C66X_COREPAC_LOCAL_L2_SRAM_REGS + 0x30000)
#define L2_PRIVATE_SRAM_END		(L2_PRIVATE_SRAM_BASE + L2_SRAM_SIZE - 1)
#define MSMC_SHARED_SRAM_BASE 	(CSL_MSMC_SRAM_REGS + 0x400)
#define MSMC_SHARED_SRAM_END	(CSL_MSMC_SRAM_REGS + MSMC_SRAM_SIZE - 1)
// FIXME: based should be moved to some offset since the network model is stored in .sharedram which is on DDR as well
#define DDR_SHARED_DRAM_BASE		(CSL_DDR3_0_DATA + 0x1000000)	// keeping a safe offset of 16MB for the network model(FC layers).
#define DDR_SHARED_DRAM_END		(CSL_DDR3_0_DATA + (DDR_RAM_SIZE - 1))

static char *p_shared_cur_free = (char *)MSMC_SHARED_SRAM_BASE;
void * shared_malloc(size_t size) {
	uint32_t no_blocks;
	char *p_new_free, *ptr;



	no_blocks = (size + MSMC_ALIGNMENT - 1) / MSMC_ALIGNMENT;
	p_new_free = p_shared_cur_free + no_blocks * MSMC_ALIGNMENT;

	if (p_new_free > (char *)MSMC_SHARED_SRAM_END) {
		REL_INFO("Cannot allocate memory\n");
		return NULL;
	}
	ptr = p_shared_cur_free;
	p_shared_cur_free = p_new_free;
	return ptr;
}

void * private_malloc(size_t size) {
	// TODO
	return NULL;
}
static char *p_ext_cur_free = (char *)DDR_SHARED_DRAM_BASE;
void * ext_malloc(size_t size) {
	uint32_t no_blocks;
	char *p_new_free, *ptr;



	no_blocks = (size + DRAM_ALIGNMENT - 1) / DRAM_ALIGNMENT;
	p_new_free = p_ext_cur_free + no_blocks * DRAM_ALIGNMENT;

	if (p_new_free > (char *)DDR_SHARED_DRAM_END) {
		REL_INFO("Cannot allocate memory\n");
		return NULL;
	}
	ptr = p_ext_cur_free;
	p_ext_cur_free = p_new_free;
	return ptr;
}

void reset_mem_manager() {
	p_shared_cur_free = (char *)MSMC_SHARED_SRAM_BASE;
	p_ext_cur_free = (char *)DDR_SHARED_DRAM_BASE;
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
