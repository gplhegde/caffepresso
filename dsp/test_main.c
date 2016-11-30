#include <stdio.h>
#include <stdint.h>
#include "debug_control.h"
#include "unit_test.h"
#include "misc_utils.h"

#ifndef DEVICE_K2H
#error "Device not specified"
#endif
#include <ti/csl/cslr_device.h>
#include <ti/csl/csl_semAux.h>
#include <ti/csl/csl_tsc.h>
#include <ti/csl/csl_cacheAux.h>
#include "c6x.h"

#pragma DATA_SECTION(core_id, ".local_ram")
// CPU ID. This is local to each core since we are storing this in local RAM.
unsigned int core_id;

#pragma DATA_SECTION(completion_cnt, ".sharedram")
unsigned int completion_cnt;

#pragma DATA_SECTION(p_flt_input, ".sharedram")
FLT_MAP *p_flt_input;

#pragma DATA_SECTION(p_fix_input, ".sharedram")
FIX_MAP *p_fix_input;

#pragma DATA_SECTION(p_ref_flt_output, ".sharedram")
FLT_MAP *p_ref_flt_output;

int test_layers();

void dsp_init() {

	// Enable timers for profiling.
	CSL_tscEnable();
	// We will not use L2 cache. We will manage L2 RAM to keep local variables specific to the core.
	CACHE_setL2Size (CACHE_0KCACHE);
	// Use L1 D cache
	CACHE_setL1DSize(CACHE_L1_32KCACHE);
	// Disable caching for starting 16MB DDR(refer to the API)
	CACHE_disableCaching (128);
}
void main(void) {

	if(DNUM == 0) {
		hSEM->SEM_RST_RUN = CSL_FMK(SEM_SEM_RST_RUN_RESET, 1);
		printf("Sem reset\n");
	}

	core_id = DNUM;

	dsp_init();

	test_layers();

	printf("Application complete\n");

	while(1);
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

	return status;
}

