/*******************************************************
* File Name -- main.c
*
* Authors:  Nachiappan
*
* Created on : 27-September-2015
*
* Contents: Deeplearn algorithm implementation
*           for TI TMS320C6678 - DSP
*
********************************************************/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <csl_tsc.h>
#include <csl_chipAux.h>
#include <tistdtypes.h>
#include <csl_types.h>
#include <csl_cacheAux.h>
#include <csl_semAux.h>
#include <csl_psc.h>
#include <csl_pscAux.h>
#include <cslr_emif4f.h>
#include <csl_emif4fAux.h>
#include "c6x.h"
#include "deep_learning.h"
#include "user_config.h"


#include <ti/csl/csl_semAux.h>

#include <img_dd_32x32.h>

#define ITERATIONS 100000

CSL_Uint64 startVal,endVal;
double cumulative;

#ifdef FUNCTION_PROFILE
CSL_Uint64 startVal1,endVal1,startVal2,endVal2;
double layer1,layer2,layer3,conv1,conv2,conv3,pool1,pool2,pool3,rect1,rect2,rect3,add1,add2,pad1,pad2,pad3;
#endif

#pragma DATA_SECTION(core_id, ".local_ram")
unsigned int core_id;

void do_power_gating()
{
	EMIF4F_PWR_MGMT_CONFIG pwrConfig;

	hPscRegs->PDCTL[CSL_PSC_PD_PCIEX] = hPscRegs->PDCTL[CSL_PSC_PD_PCIEX]& 0xFFFF0FFF;
	CSL_PSC_disablePowerDomain (CSL_PSC_PD_PCIEX);
    CSL_PSC_setModuleNextState (CSL_PSC_LPSC_PCIEX, PSC_MODSTATE_SWRSTDISABLE);

    /* Start the state transition */
    CSL_PSC_startStateTransition (CSL_PSC_PD_PCIEX);

    /* Wait until the state transition process is completed. */
    while (!CSL_PSC_isStateTransitionDone (CSL_PSC_PD_PCIEX));

	hPscRegs->PDCTL[CSL_PSC_PD_SRIO] = hPscRegs->PDCTL[CSL_PSC_PD_SRIO]& 0xFFFF0FFF;
	CSL_PSC_disablePowerDomain (CSL_PSC_PD_SRIO);
    CSL_PSC_setModuleNextState (CSL_PSC_LPSC_SRIO, PSC_MODSTATE_SWRSTDISABLE);

    /* Start the state transition */
    CSL_PSC_startStateTransition (CSL_PSC_PD_SRIO);

    /* Wait until the state transition process is completed. */
    while (!CSL_PSC_isStateTransitionDone (CSL_PSC_PD_SRIO));

	hPscRegs->PDCTL[CSL_PSC_PD_HYPERBRIDGE] = hPscRegs->PDCTL[CSL_PSC_PD_HYPERBRIDGE]& 0xFFFF0FFF;
	CSL_PSC_disablePowerDomain (CSL_PSC_PD_HYPERBRIDGE);
    CSL_PSC_setModuleNextState (CSL_PSC_LPSC_HYPERBRIDGE, PSC_MODSTATE_SWRSTDISABLE);

    /* Start the state transition */
    CSL_PSC_startStateTransition (CSL_PSC_PD_HYPERBRIDGE);

    /* Wait until the state transition process is completed. */
    while (!CSL_PSC_isStateTransitionDone (CSL_PSC_PD_HYPERBRIDGE));

	hPscRegs->PDCTL[CSL_PSC_PD_PASS] = hPscRegs->PDCTL[CSL_PSC_PD_PASS]& 0xFFFF0FFF;
	CSL_PSC_disablePowerDomain (CSL_PSC_PD_PASS);
    CSL_PSC_setModuleNextState (CSL_PSC_LPSC_PKTPROC, PSC_MODSTATE_SWRSTDISABLE);
    CSL_PSC_setModuleNextState (CSL_PSC_LPSC_CPGMAC,  PSC_MODSTATE_SWRSTDISABLE);
    CSL_PSC_setModuleNextState (CSL_PSC_LPSC_Crypto,  PSC_MODSTATE_SWRSTDISABLE);

    /* Start the state transition */
    CSL_PSC_startStateTransition (CSL_PSC_PD_PASS);

    /* Wait until the state transition process is completed. */
    while (!CSL_PSC_isStateTransitionDone (CSL_PSC_PD_PASS));

	CSL_PSC_setModuleNextState (CSL_PSC_LPSC_EMIF25_SPI, PSC_MODSTATE_SWRSTDISABLE);
    CSL_PSC_startStateTransition (CSL_PSC_LPSC_EMIF25_SPI);
    while (!CSL_PSC_isStateTransitionDone (CSL_PSC_LPSC_EMIF25_SPI));

	CSL_PSC_setModuleNextState (CSL_PSC_LPSC_TSIP, PSC_MODSTATE_SWRSTDISABLE);
    CSL_PSC_startStateTransition (CSL_PSC_LPSC_TSIP);
    while (!CSL_PSC_isStateTransitionDone (CSL_PSC_LPSC_TSIP));

//	CSL_PSC_setModuleNextState (CSL_PSC_LPSC_DEBUGSS_TRC, PSC_MODSTATE_SWRSTDISABLE);
//    CSL_PSC_startStateTransition (CSL_PSC_LPSC_DEBUGSS_TRC);
//    while (!CSL_PSC_isStateTransitionDone (CSL_PSC_LPSC_DEBUGSS_TRC));

    hPscRegs->PDCTL[CSL_PSC_PD_GEM0] = (hPscRegs->PDCTL[CSL_PSC_PD_GEM0]& 0xFFFF0FFF);
    hPscRegs->PDCTL[CSL_PSC_PD_GEM1] = (hPscRegs->PDCTL[CSL_PSC_PD_GEM1]& 0xFFFF0FFF);
    hPscRegs->PDCTL[CSL_PSC_PD_GEM2] = (hPscRegs->PDCTL[CSL_PSC_PD_GEM2]& 0xFFFF0FFF);
    hPscRegs->PDCTL[CSL_PSC_PD_GEM3] = (hPscRegs->PDCTL[CSL_PSC_PD_GEM3]& 0xFFFF0FFF);
    hPscRegs->PDCTL[CSL_PSC_PD_GEM4] = (hPscRegs->PDCTL[CSL_PSC_PD_GEM4]& 0xFFFF0FFF);
    hPscRegs->PDCTL[CSL_PSC_PD_GEM5] = (hPscRegs->PDCTL[CSL_PSC_PD_GEM5]& 0xFFFF0FFF);
    hPscRegs->PDCTL[CSL_PSC_PD_GEM6] = (hPscRegs->PDCTL[CSL_PSC_PD_GEM6]& 0xFFFF0FFF);
    hPscRegs->PDCTL[CSL_PSC_PD_GEM7] = (hPscRegs->PDCTL[CSL_PSC_PD_GEM7]& 0xFFFF0FFF);

    hPscRegs->MDCTL[CSL_PSC_LPSC_EMIF4F] = (hPscRegs->MDCTL[CSL_PSC_LPSC_EMIF4F] | (~0xFFFEFFF));
    CSL_EMIF4F_GetPowerMgmtConfig(&pwrConfig);
    pwrConfig.lpMode = 2;
    CSL_EMIF4F_SetPowerMgmtConfig(&pwrConfig);

//    switch(NUM_CORES)
//    {
//    case 0: CSL_PSC_disablePowerDomain (CSL_PSC_PD_GEM0);
//    case 1: CSL_PSC_disablePowerDomain (CSL_PSC_PD_GEM1);
//    case 2: CSL_PSC_disablePowerDomain (CSL_PSC_PD_GEM2);
//    case 3: CSL_PSC_disablePowerDomain (CSL_PSC_PD_GEM3);
//    case 4: CSL_PSC_disablePowerDomain (CSL_PSC_PD_GEM4);
//    case 5: CSL_PSC_disablePowerDomain (CSL_PSC_PD_GEM5);
//    case 6: CSL_PSC_disablePowerDomain (CSL_PSC_PD_GEM6);
//    case 7: CSL_PSC_disablePowerDomain (CSL_PSC_PD_GEM7);
//    default:
//    	break;
//    }
}

compute_num_maps()
{
	int num_core;
	maps_info_ptr->mod_l1 = L1_MAPS%NUM_CORES;
	maps_info_ptr->mod_l2 = L2_MAPS%NUM_CORES;
	maps_info_ptr->mod_l3 = L3_MAPS%NUM_CORES;

	for(num_core=0;num_core<NUM_CORES;num_core++)
	{
		if(!(L1_MAPS%NUM_CORES))
			maps_info_ptr->l1_maps[num_core]= L1_MAPS/NUM_CORES;
		else if(num_core<(L1_MAPS%NUM_CORES))
			maps_info_ptr->l1_maps[num_core]= (L1_MAPS/NUM_CORES)+1;
		else
			maps_info_ptr->l1_maps[num_core]= L1_MAPS/NUM_CORES;

		if(!(L2_MAPS%NUM_CORES))
			maps_info_ptr->l2_maps[num_core]= L2_MAPS/NUM_CORES;
		else if(num_core<(L2_MAPS%NUM_CORES))
			maps_info_ptr->l2_maps[num_core]= (L2_MAPS/NUM_CORES)+1;
		else
			maps_info_ptr->l2_maps[num_core]= L2_MAPS/NUM_CORES;

		if(!(L3_MAPS%NUM_CORES))
			maps_info_ptr->l3_maps[num_core]= L3_MAPS/NUM_CORES;
		else if(num_core<(L3_MAPS%NUM_CORES))
			maps_info_ptr->l3_maps[num_core]= (L3_MAPS/NUM_CORES)+1;
		else
			maps_info_ptr->l3_maps[num_core]= L3_MAPS/NUM_CORES;
	}
}
void main()
{
	int i,img_width=32,img_height=32;

	core_id = DNUM;

	CSL_tscEnable();

	CACHE_setL2Size (CACHE_0KCACHE);
	CACHE_setL1DSize(CACHE_L1_32KCACHE);
	CACHE_disableCaching (128);

	maps_info_ptr = (maps_info*)MAPS_INFO_PTR;

	if(DNUM==MASTER_CORE_ID)
	{
		CSL_semAcquireDirect(INIT_DONE_SEM);
		memset((void*)MSMC_REG_BASE,0x0,MSMC_SRAM_SIZE);
		memset((void*)MAPS_INFO_PTR,0x0,0x100);
		do_power_gating();
		compute_num_maps();
	}

	memset((void*)L2_HEAP_BASE,0x0,L2_HEAP_SIZE);

	for(i=0;i<ITERATIONS;i++)
	{
		startVal = _itoll(TSCH,TSCL);
		deeplearn(in_img, img_width, img_height);
		endVal = _itoll(TSCH,TSCL);
		cumulative += ((endVal-startVal)/DSP_FREQ_IN_MHZ);
	}
	if(DNUM==MASTER_CORE_ID)
	{
#ifdef FUNCTION_PROFILE
		printf("%lf %lf %lf \n",(layer1/ITERATIONS),(layer2/ITERATIONS),(layer3/ITERATIONS));
		printf("%lf %lf %lf %lf \n",(pad1/ITERATIONS),(conv1/ITERATIONS),(rect1/ITERATIONS),(pool1/ITERATIONS));
		printf("%lf %lf %lf %lf %lf \n",(add1/ITERATIONS),(pad2/ITERATIONS),(conv2/ITERATIONS),(rect2/ITERATIONS),(pool2/ITERATIONS));
		printf("%lf %lf %lf %lf %lf \n",(add2/ITERATIONS),(pad3/ITERATIONS),(conv3/ITERATIONS),(rect3/ITERATIONS),(pool3/ITERATIONS));
#else
		printf("%lf us",(cumulative/ITERATIONS));
#endif
	}
	cumulative=0;
}

#if 0
void delaycycles(uint32_t cycles)
{
	uint32_t start_val  = CSL_chipReadTSCL();

	while ((CSL_chipReadTSCL() - start_val) < cycles);

	return;
}
#endif
