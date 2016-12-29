/*
 * main.c
 */
#include <stdint.h>
#include <stdio.h>
#include "fast_gemm.h"
#include <ti/csl/csl_cacheAux.h>
#include <ti/csl/csl_tsc.h>
#include <ti/csl/csl_chip.h>
#include "data_sync.h"
#include "config.h"
#include "edma_module.h"

unsigned int core_id;
void dsp_init() {

	// Enable timers for profiling.
	CSL_tscEnable();

#ifdef USE_L2_CACHE
	CACHE_setL2Size (CACHE_512KCACHE);
#else
	// We will not use L2 cache. We will manage L2 RAM to keep local variables specific to the core.
	CACHE_setL2Size (CACHE_0KCACHE);
#endif // USE_L2_CACHE

	// Use L1 D cache
	CACHE_setL1DSize(CACHE_L1_16KCACHE);

	// Set cache write-through mode for MSMC RAM region
	CACHE_setMemRegionWritethrough(12, TRUE);
}

int main(void) {

	core_id = CSL_chipReadReg(CSL_CHIP_DNUM);

	get_global_sync_obj();

	if(core_id != MASTER_CORE_ID) {
		REL_INFO("C_%d : Waiting for global config to finish\n", core_id);
		wait_global_config();
		L1_CACHE_INV((void *)shared_edma_obj, sizeof(shared_edma_obj), CACHE_WAIT);
	}

	// init core specific HW
	dsp_init();


	if(core_id == MASTER_CORE_ID) {
		// Reset semaphore module
		hSEM->SEM_RST_RUN = CSL_FMK(SEM_SEM_RST_RUN_RESET, 1);
		// Init all EDMA channels for all cores.
		all_edma_init();
		// init global sync object and tell other cores to go and perform local init
		flag_global_config_done();
	}

	flag_local_config_done();

	wait_all_local_config();
	//run_gemm_bmark();
	run_fast_gemm_bmark();
	return 0;
}
