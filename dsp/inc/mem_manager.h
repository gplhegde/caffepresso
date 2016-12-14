/*
 * mem_manager.h
 *
 *  Created on: 18 Nov 2016
 *      Author: Gopalakrishna Hegde
 */

#ifndef _MEM_MANAGER_H_
#define _MEM_MANAGER_H_
#include <stdio.h>

#define L2_SRAM_ALIGNMENT CACHE_L1D_LINESIZE
#define MSMC_ALIGNMENT CACHE_L1D_LINESIZE
#define DRAM_ALIGNMENT CACHE_L1D_LINESIZE

// size of private temp buffer used for padding and alignment during CONV and IP layers
#define PRIVATE_TEMP_BUFF_SIZE	4*1024

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

void * shared_malloc(size_t size);
void * private_malloc(size_t size);
void * ext_malloc(size_t size);
void reset_mem_manager();
void shared_free(void *ptr);
void private_free(void *ptr);
void ext_free(void *ptr);
#endif /* INC_MEM_MANAGER_H_ */
