/*  ============================================================================
 *   Copyright (c) Texas Instruments Inc 2002, 2003, 2004, 2005
 *
 *   Use of this software is controlled by the terms and conditions found in the
 *   license agreement under which this software has been supplied.
 *  ============================================================================
 */



/**********************************************************
 **********************************************************/
-c
-stack 0x1000
-heap  0x1000

MEMORY
{

    MSMC_RAM:           o = 0x0C000000   l = 0x000ec000  /* 1Mb alloc for each core */
    DDR3_RAM:           o = 0x80000000   l = 0x40000000
    L1_SRAM:            o = 0x00f00000   l = 0x00004000 /*16KB configured as SRAM and 16kB L1D cache*/
    DSP_L2:             o = 0x10800000   l = 0x00080000
}
 
SECTIONS
{

    .text       >       DSP_L2
    .stack      >       DSP_L2
    .bss        >       DSP_L2
    .cinit      >       DSP_L2
    .cio        >       DSP_L2
    .const      >       DSP_L2
    .data       >       DSP_L2
    .neardata   >       DSP_L2
    .rodata     >       DSP_L2
    .switch     >       DSP_L2
    .sysmem     >       DSP_L2
    .far        >       DDR3_RAM
    .fardata    >       DSP_L2
    .testMem    >       DSP_L2
	.CSL_VARS	>		DSP_L2
	.inMSMC_RAM	>	    DSP_L2
    .farData    >       DDR3_RAM
    .l2_data    >       DSP_L2
    .ddr_data  	>       DDR3_RAM
    .l1_data	> 		L1_SRAM
}

/*****************************************************************************/
/*                       End of Linker command file                          */
/*****************************************************************************/
