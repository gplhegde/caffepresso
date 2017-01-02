#include <stdio.h>
#include <stdint.h>
#include "debug_control.h"
#include "unit_test.h"
#include "misc_utils.h"
#include "data_sync.h"
#include "dsp_bmarks.h"
#include "edma_module.h"
#include "mem_manager.h"
#ifndef DEVICE_K2H
#error "Device not specified"
#endif
#include <ti/csl/cslr_device.h>
#include <ti/csl/csl_semAux.h>
#include <ti/csl/csl_tsc.h>
#include <ti/csl/csl_chip.h>
#include <ti/csl/csl_cacheAux.h>
#include "c6x.h"

// unedf this to run the layer tests.
#define RUN_BMARKS 0

//#pragma DATA_SECTION(core_id, ".local_ram") // Disabling this as we are putting entire code is local ram
// CPU ID. This is local to each core since we are storing this in local RAM.
unsigned int core_id;


#pragma DATA_SECTION(completion_cnt, ".shared_ocm")
unsigned int completion_cnt[2];

#pragma DATA_SECTION(p_flt_input, ".shared_ocm")
FLT_MAP *p_flt_input;

#pragma DATA_SECTION(p_fix_input, ".shared_ocm")
FIX_MAP *p_fix_input;

#pragma DATA_SECTION(p_ref_flt_output, ".shared_ocm")
FLT_MAP *p_ref_flt_output;

int test_layers();

void dsp_init() {

	// Enable timers for profiling.
	CSL_tscEnable();

	// We will not use L2 cache. We will manage L2 RAM to keep local variables specific to the core.
	CACHE_setL2Size (CACHE_0KCACHE);

	// Use L1 D cache fully
	CACHE_setL1DSize(CACHE_L1_32KCACHE);

	// Disable caching for starting 16MB DDR(refer to the API)
	CACHE_disableCaching (128);

	// Setting write through mode for MSMC RAM for time being instead of disabling the L1D cache completely.
	CACHE_setMemRegionWritethrough(12, TRUE);
}

void main(void) {
	core_id = CSL_chipReadReg(CSL_CHIP_DNUM);

	// init core specific HW
	dsp_init();
	get_global_sync_obj();

	if(core_id != MASTER_CORE_ID) {
		REL_INFO("C_%d : Waiting for global config to finish\n", core_id);
		wait_global_config();
		L1_CACHE_INV((void *)CSL_MSMC_SRAM_REGS, MSMC_SRAM_SIZE, CACHE_WAIT);
	}

	if(core_id == MASTER_CORE_ID) {
		// Reset semaphore module
		hSEM->SEM_RST_RUN = CSL_FMK(SEM_SEM_RST_RUN_RESET, 1);

		// Setup EDMA channels for all cores
		all_edma_init();
		// init global sync object and tell other cores to go and perform local init
		flag_global_config_done();
	}

	flag_local_config_done();

	wait_all_local_config();

#if RUN_BMARKS
	// run benchmarks
	run_dsp_bmarks();
#else
	// test all supported layers
	//test_layers();
	conv_layer_sweep();
#endif
	printf("%d : Application complete\n", core_id);
}

int test_layers() {
	TEST_STATUS_E status;

	if(DNUM == 0) {
		while(!CSL_semAcquireDirect(INIT_DONE_SEM));
	} else {
		// This wait is to prevent other cores proceed before the master core acquires the semaphore.
		// These cores will proceed once the master acquires semaphore to perform init. Again they wait for the init to complete.
		while(CSL_semIsFree(INIT_DONE_SEM));
	}

	printf("--------Testing CONV Layer-------\n");
	status = test_conv_layer();
	if(status != TEST_PASS) {
		REL_INFO("Conv layer test failed\nError = %d\n", status);
		REL_INFO("Aborting...\n");
		return -1;
	}

	if(DNUM == 0) {
		while(!CSL_semAcquireDirect(INIT_DONE_SEM));
	} else {
		while(CSL_semIsFree(INIT_DONE_SEM));
	}

	printf("--------Testing POOL Layer-------\n");
	status = test_pool_layer();
	if(status != TEST_PASS) {
		REL_INFO("Pool layer test failed\nError = %d\n", status);
		REL_INFO("Aborting...\n");
		return -1;
	}

	if(DNUM == 0) {
		while(!CSL_semAcquireDirect(INIT_DONE_SEM));
	} else {
		while(CSL_semIsFree(INIT_DONE_SEM));
	}
	printf("--------Testing FC Layer-------\n");
	status = test_ip_layer();
	if(status != TEST_PASS) {
		REL_INFO("Inner product layer test failed\nError = %d\n", status);
		REL_INFO("Aborting...\n");
		return -1;
	}

	if(DNUM == 0) {
		while(!CSL_semAcquireDirect(INIT_DONE_SEM));
	} else {
		while(CSL_semIsFree(INIT_DONE_SEM));
	}
	printf("--------Testing Batch Normalization Layer-------\n");
	status = test_bnorm_layer();
	if(status != TEST_PASS) {
		REL_INFO("Batch Normalization layer test failed\nError = %d\n", status);
		REL_INFO("Aborting...\n");
		return -1;
	}

	return status;
}

