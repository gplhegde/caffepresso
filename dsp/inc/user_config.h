/*
 * user_config.h
 *
 *  Created on: Sep 30, 2015
 *      Author: Nachiappan
 */

#ifndef _USER_CONFIG_H
#define _USER_CONFIG_H

// Enable this for profiling info
#define FUNCTION_PROFILE

// TODO: Get these directly from TI platform specific header files.
#ifndef __KEYSTONE1

#define DSP_FREQ_IN_MHZ			1400
#define NO_CORES 				8
#define MSMC_REG_BASE			0x0C000000
#define MSMC_SRAM_SIZE 			(6*1024*1024)
#define L2_HEAP_BASE			0x00820000
#define L2_HEAP_SIZE			(896*1024)    //scratch space in L2

#define DDR_BASE				0x81000000
#define DDR_SIZE				0x00A00000

#else
#define DSP_FREQ_IN_MHZ			1000
#define NUM_CORES 				8
#define MSMC_REG_BASE			0x0C000000
#define MSMC_SRAM_SIZE 			(4*1024*1024)
#define L2_HEAP_BASE			0x0081C000
#define L2_HEAP_SIZE			(400*1024)    //400KB scratch space in L2

#define DDR_BASE				0x81000000
#define DDR_SIZE				0x04000000
#endif

// This core is responsible for all init
#define MASTER_CORE_ID			0
// HW semaphores for shared data coherency and synchronization
#define INIT_DONE_SEM	 		2

// Location of the shared data structure. This belongs to MSMC RAM address space.
#define MAPS_INFO_PTR 0x80000000

#endif /*_USER_CONFIG_H */
