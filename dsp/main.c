#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "user_config.h"
#include "debug_control.h"
#include "cnn_app.h"
#include "app_init.h"
#include "misc_utils.h"
#include "caffe_proto_params.h"
#include "data_sync.h"

#ifndef DEVICE_K2H
#error "Device not specified"
#endif
#include <ti/csl/cslr_device.h>
#include <ti/csl/csl_semAux.h>
#include <ti/csl/csl_tsc.h>
#include <ti/csl/csl_chip.h>
#include <ti/csl/csl_cacheAux.h>
#include "c6x.h"

#pragma DATA_SECTION(core_id, ".local_ram")
// CPU ID. This is local to each core since we are storing this in local RAM.
unsigned int core_id;

#define PRINT_WORKLOAD_PARAMS	(0)

void dsp_init() {

	// Enable timers for profiling.
	CSL_tscEnable();

	// We will not use L2 cache. We will manage L2 RAM to keep local variables specific to the core.
	CACHE_setL2Size (CACHE_0KCACHE);

	// Use L1 D cache
	CACHE_setL1DSize(CACHE_L1_32KCACHE);

	// Disable caching for starting 16MB DDR(refer to the API)
	CACHE_disableCaching (128);

	// Set cache write-through mode for MSMC RAM region
	CACHE_setMemRegionWritethrough(12, TRUE);
}

void main_init() {
	// Reset the MSMC RAM
	//memset((void*)MSMC_REG_BASE, 0x0, MSMC_SRAM_SIZE);

	// Init the main framework
	main_cnn_app_init();
}

int main() {
	uint8_t *p_image;
	uint32_t label, img_width, img_height;

	core_id = CSL_chipReadReg(CSL_CHIP_DNUM);

	get_global_sync_obj();

	if(core_id != MASTER_CORE_ID) {
		REL_INFO("C_%d : Waiting for global config to finish\n", core_id);
		wait_global_config();
	}

	// init core specific HW
	dsp_init();
	

	if(core_id == MASTER_CORE_ID) {
		// Reset semaphore module
		hSEM->SEM_RST_RUN = CSL_FMK(SEM_SEM_RST_RUN_RESET, 1);

		// CNN specific shared context init
		main_init();

		// init global sync object and tell other cores to go and perform local init
		flag_global_config_done();
	}

	flag_local_config_done();

	wait_all_local_config();

	if(PRINT_WORKLOAD_PARAMS) {
		print_layer_node_ctx(g_cnn_layer_nodes, NO_DEEP_LAYERS);
	}

	// TODO: Do  Input image init, normalization. Only by the master core.

	// run the main application
	main_cnn_app(p_image, &label);


	if(core_id == MASTER_CORE_ID) {
		REL_INFO("C_%d : Relesaing resources\n", core_id);
		// Release all resources and cleanup
		cnn_app_memfree();
	}

	REL_INFO("C_%d : Application complete\n", core_id);

	return 0;
}





