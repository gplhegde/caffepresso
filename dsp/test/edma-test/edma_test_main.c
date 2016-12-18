

#include <edma_test.h>
#include <ti/csl/soc.h>
#include <ti/csl/tistdtypes.h>
#include <ti/csl/csl_chip.h>
#include <ti/csl/csl_edma3.h>
#include <ti/csl/csl_edma3Aux.h>
#include <ti/csl/cslr_spi.h>
#include <ti/csl/csl_cacheAux.h>
#include <stdio.h>
#include <stdlib.h>

#pragma DATA_SECTION(dstBuf, ".l2_data")
#pragma DATA_ALIGN(dstBuf, 8)
Uint32 dstBuf[BUF_SIZE];

#pragma DATA_SECTION(srcBuf, ".ddr_data")
#pragma DATA_ALIGN(srcBuf, 8)
Uint32 srcBuf[BUF_SIZE];

CSL_Edma3Handle hModule;

CSL_Edma3ParamHandle paramHandle0;

CSL_Edma3ChannelAttr  chParam;

CSL_Edma3ChannelObj  ChObj0;

CSL_Edma3ChannelHandle  hChannel0;

CSL_Edma3HwDmaChannelSetup chSetup;

CSL_Edma3ParamSetup paramSetup;

CSL_Edma3Obj  moduleObj;

CSL_Edma3CmdIntr regionIpr;

CSL_Status EdmaStat;


void trigger_edma_channel(void){
	int bcnt;
	if(EDMA_SYNC_MODE == CSL_EDMA3_SYNC_A) {
		// Need to send BCNT no of events if the transfer mode is A-synchronous.
		for(bcnt = 0; bcnt < TEST_BCNT; bcnt++) {
			// set the DMA event to start the transfer
			CSL_edma3HwChannelControl(hChannel0, CSL_EDMA3_CMD_CHANNEL_SET, NULL);
		}
	} else {
		CSL_edma3HwChannelControl(hChannel0, CSL_EDMA3_CMD_CHANNEL_SET, NULL);
	}
}


void setup_edma(Uint32 srcBuf,Uint32 dstBuf) {

    // EDMA Module Initialization
	CSL_edma3Init(NULL);

 	// Open EDMA Module
    hModule = CSL_edma3Open(&moduleObj, EDMA_INSTANCE_NO, NULL,&EdmaStat);
    if(!hModule) {
    	printf("Could not get the EDMA module\n");
    	exit(0);
    }

    // Open chanel in the global region.
	chParam.regionNum  = CSL_EDMA3_REGION_GLOBAL;
	chSetup.que        = CSL_EDMA3_QUE_1;
	chParam.chaNum     = EDMA_CHANEL_NO;
	hChannel0 = CSL_edma3ChannelOpen(&ChObj0, EDMA_INSTANCE_NO, &chParam, &EdmaStat);

	// Assign the PaRAM Set to this chanel
	chSetup.paramNum   = EDMA_CHANEL_PARAM_NO;
    CSL_edma3HwChannelSetupParam(hChannel0, chSetup.paramNum);

   	// Enable the chanel. This has no effect for manual trigger mode from DSP by writing to ESR
	//CSL_edma3HwChannelControl(hChannel0, CSL_EDMA3_CMD_CHANNEL_DISABLE, NULL);
   	//CSL_edma3HwChannelControl(hChannel0, CSL_EDMA3_CMD_CHANNEL_ENABLE,NULL);

    // Setup the event queue for the chanel.
    CSL_edma3HwChannelSetupQue(hChannel0, chSetup.que);

	// Parameter Handle Open
	// Get the handle to the parameter set(address to the PaRAM with proper offset)
	paramHandle0            = CSL_edma3GetParamHandle(hChannel0, chSetup.paramNum, &EdmaStat);

	// Populate the EDMA transfer parameters
    paramSetup.aCntbCnt     = CSL_EDMA3_CNT_MAKE(TEST_ACNT * 4,(TEST_BCNT));
	paramSetup.srcDstBidx   = CSL_EDMA3_BIDX_MAKE(1, 1);
	paramSetup.srcDstCidx   = CSL_EDMA3_CIDX_MAKE(1, 1);
	paramSetup.cCnt         = TEST_CCNT;
	paramSetup.option       = CSL_EDMA3_OPT_MAKE(FALSE,FALSE,FALSE,TRUE,0 ,CSL_EDMA3_TCC_NORMAL, \
	      CSL_EDMA3_FIFOWIDTH_NONE, FALSE, EDMA_SYNC_MODE, CSL_EDMA3_ADDRMODE_INCR, CSL_EDMA3_ADDRMODE_INCR);
	paramSetup.srcAddr      = (Uint32)(srcBuf);
	paramSetup.dstAddr      = (Uint32)(dstBuf);
	paramSetup.linkBcntrld  = CSL_EDMA3_LINKBCNTRLD_MAKE(CSL_EDMA3_LINK_NULL,1);

	// Write the transfer parameters to the PaRAM
	CSL_edma3ParamSetup(paramHandle0,&paramSetup);

   	// TODO: clear only if SER bits are set
   	// clear secondary event set reg for this chanel
	CSL_edma3ClearDMAChannelSecondaryEvents(hModule, chParam.chaNum);

   	// clear any missed events for this chanel
   	// TODO: clear only if set
	CSL_edma3ClearDMAMissedEvent(hModule, chParam.chaNum);

   	// Enable the interrupt. This is necessary only if the interrupt to be forwarded to DSP, else not required.
    regionIpr.region  = CSL_EDMA3_REGION_GLOBAL;
	regionIpr.intr    = 1;
	regionIpr.intrh   = 0;
   	CSL_edma3HwControl(hModule, CSL_EDMA3_CMD_INTR_ENABLE, &regionIpr);
}

void wait_for_completion(void) {

	// Wait for interrupt
    regionIpr.region  = CSL_EDMA3_REGION_GLOBAL;
	regionIpr.intr    = 0;
	regionIpr.intrh   = 0;
	printf("Waiting for interrupt to be pending\n");

	do{
		// waiting for the bit (= TCC number) interrupt pending register to be set.
		CSL_edma3GetHwStatus(hModule, CSL_EDMA3_QUERY_INTRPEND, &regionIpr);

	}while ((regionIpr.intr & (1 << 0)) != (1 << 0));	//since TCC code is set to 0, wait for bit 0 in IPR

	// clear interrupt
	CSL_edma3HwControl(hModule, CSL_EDMA3_CMD_INTRPEND_CLEAR, &regionIpr);

	printf("Transfer is complete\n");

}

void close_edma() {
    CSL_FINST(hModule->regs->TPCC_SECR,TPCC_TPCC_SECR_SECR2,RESETVAL);
  	CSL_FINST(hModule->regs->TPCC_SECR,TPCC_TPCC_SECR_SECR3,RESETVAL);

 	CSL_edma3ChannelClose(hChannel0);
 	CSL_edma3Close(hModule);
}

Bool verify_transfer(Uint32  *p_src, Uint32 *p_dst, int N)
{
	int i;
	Bool status = TRUE;
	for (i = 0; i < N; i++)	{
		if (*p_src++ != *p_dst++) {
			status = FALSE;
		    break ;
		}
	}

	return status;
}

void init_buffers(Uint32 *p_src, Uint32 *p_dst, int N) {
	int i;
	for (i = 0; i < N; i++) {
		*p_src++ = (Uint32) rand();
		*p_dst++ = 0xABCDABCD;
	}
}

void main (void) {

	// Init source buff to random value and dst buffer to a fixed pattern.
	init_buffers(srcBuf, dstBuf, BUF_SIZE);

   	//Setup EDMA memory to memory transfer.
	setup_edma((Uint32)srcBuf, (Uint32)dstBuf);

   	// Trigger EMDA transfer by setting an event to the chanel. This is manual trigger method from DSP.
	trigger_edma_channel();

	//Wait for DMA completion. This is polling method.
   	wait_for_completion();

	// Close DMA objects.                                        */
    close_edma();

	if(verify_transfer(srcBuf, dstBuf, BUF_SIZE)) {
		printf("EDMA TEST PASS\n");
	} else {
		printf("EDMA TEST FAILED\n");
	}

	printf("----------EDMA Test Complete--------\n");
}
