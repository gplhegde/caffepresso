#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "user_config.h"
#include "debug_control.h"
#include "cnn_app.h"
#include "caffe_proto_params.h"

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
	memset((void*)L2_HEAP_BASE, 0x0, L2_HEAP_SIZE);
}

void main_init() {

	memset((void*)MSMC_REG_BASE, 0x0, MSMC_SRAM_SIZE);
	
	main_cnn_app_init();
	
	CSL_semReleaseSemaphore(INIT_DONE_SEM);
}

int main() {
	uint8_t *p_image;
	int label, img_width, img_height;
	
	core_id = DNUM;
	
	// init core specific HW
	dsp_init();
	
	// perform main DNN framework and context init.
	// Only master core is going to perform this.
	if(core_id == MASTER_CORE_ID) {
		while(CSL_semAcquireDirect(INIT_DONE_SEM) != 0);
		main_init();
		CSL_semReleaseSemaphore(INIT_DONE_SEM);
	}

	// wait for all init to get over.
	while(!CSL_semIsFree(INIT_DONE_SEM));
	//main_cnn_app(pImage, &label);

	printf("Application complete\n");

	return 0;
}





