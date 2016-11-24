#include <stdio.h>
#include "debug_control.h"
#include "unit_test.h"
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
	// L2 RAM is local to the core. Hence each core will clear their own L2 RAMs
	//memset((void*)L2_HEAP_BASE, 0x0, L2_HEAP_SIZE);
}
void main(void) {
	dsp_init();

	if(DNUM == 0) {
		test_layers();
		printf("Application complete\n");
	}

	while(1);

}


int test_layers() {
	TEST_STATUS_E status;
	printf("--------Testing POOL Layer-------\n");
	status = test_pool_layer();
	if(status != TEST_PASS) {
		REL_INFO("Pool layer test failed\nError = %d\n", status);
		REL_INFO("Aborting...\n");
		return -1;
	}
	/*
	printf("--------Testing CONV Layer-------\n");
	status = test_conv_layer();
	if(status != TEST_PASS) {
		REL_INFO("Conv layer test failed\nError = %d\n", status);
		REL_INFO("Aborting...\n");
		return -1;
	}*/
	printf("--------Testing FC Layer-------\n");
	status = test_ip_layer();
	if(status != TEST_PASS) {
		REL_INFO("Inner product layer test failed\nError = %d\n", status);
		REL_INFO("Aborting...\n");
		return -1;
	}
	return status;
}

