/****************************************************************************/
/*  C6678_unified.cmd                                                       */
/*  Copyright (c) 2011 Texas Instruments Incorporated                       */
/*  Author: Rafael de Souza                                                 */
/*                                                                          */
/*    Description: This file is a sample linker command file that can be    */
/*                 used for linking programs built with the C compiler and  */
/*                 running the resulting .out file on an C6678              */
/*                 device.  Use it as a guideline.  You will want to        */
/*                 change the memory layout to match your specific C6xxx    */
/*                 target system.  You may want to change the allocation    */
/*                 scheme according to the size of your program.            */
/*                                                                          */
/****************************************************************************/

MEMORY
{

#ifndef __KEYSTONE1
    LOCAL_L2_SRAM:  o = 0x00800000 l = 0x00100000   /* 1MB LOCAL L2/SRAM */
    SHRAM:          o = 0x0C000000 l = 0x00600000   /* 6MB Multicore shared Memmory */
    DDR3:           o = 0x80000000 l = 0x80000000   /* 2GB CE0 and CE1 external DDR3 SDRAM */
#else
    LOCAL_L2_SRAM:  o = 0x00800000 l = 0x00080000   /* 512KB LOCAL L2/SRAM */
    SHRAM:          o = 0x0C000000 l = 0x00400000   /* 4MB Multicore shared Memmory */
    DDR3:           o = 0x80000000 l = 0x20000000   /* 512MB DDR3 SDRAM */
#endif
}
 
SECTIONS
{
    .kernel: {
       *.obj (.text:optimized) { SIZE(_kernel_size) }
    } load > LOCAL_L2_SRAM

    .text: load >> LOCAL_L2_SRAM
    .text:touch: load >> LOCAL_L2_SRAM

    GROUP (NEAR_DP)
    {
    .neardata
    .rodata
    .bss
    } load > LOCAL_L2_SRAM

    .far: load >> LOCAL_L2_SRAM
    .fardata: load >> LOCAL_L2_SRAM
    .data: load >> LOCAL_L2_SRAM
    .switch: load >> LOCAL_L2_SRAM
    .stack: load > LOCAL_L2_SRAM
    .args: load > LOCAL_L2_SRAM align = 0x4, fill = 0 {_argsize = 0x200; }
    .sysmem: load > LOCAL_L2_SRAM
    .cinit: load > LOCAL_L2_SRAM
    .const: load > LOCAL_L2_SRAM START(const_start) SIZE(const_size)
    .pinit: load > LOCAL_L2_SRAM
    .cio: load >> LOCAL_L2_SRAM

    .ppinfo        >  LOCAL_L2_SRAM
    .ppdata        >  LOCAL_L2_SRAM
  
    /* EABI sections */
    .binit         >  LOCAL_L2_SRAM
    .init_array    >  LOCAL_L2_SRAM
    .c6xabi.exidx  >  LOCAL_L2_SRAM
    .c6xabi.extab  >  LOCAL_L2_SRAM

    .local_ram	   >  LOCAL_L2_SRAM
    .sharedram	   >  DDR3
}
