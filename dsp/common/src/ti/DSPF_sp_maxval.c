/* ======================================================================= */
/* DSPF_sp_maxval.c -- Max Value                                           */
/*              Optimized C Implementation (w/ Intrinsics)                 */
/*                                                                         */
/* Rev 0.0.1                                                               */
/*                                                                         */
/* Copyright (C) 2011 Texas Instruments Incorporated - http://www.ti.com/  */ 
/*                                                                         */
/*                                                                         */
/*  Redistribution and use in source and binary forms, with or without     */
/*  modification, are permitted provided that the following conditions     */
/*  are met:                                                               */
/*                                                                         */
/*    Redistributions of source code must retain the above copyright       */
/*    notice, this list of conditions and the following disclaimer.        */
/*                                                                         */
/*    Redistributions in binary form must reproduce the above copyright    */
/*    notice, this list of conditions and the following disclaimer in the  */
/*    documentation and/or other materials provided with the               */
/*    distribution.                                                        */
/*                                                                         */
/*    Neither the name of Texas Instruments Incorporated nor the names of  */
/*    its contributors may be used to endorse or promote products derived  */
/*    from this software without specific prior written permission.        */
/*                                                                         */
/*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS    */
/*  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT      */
/*  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR  */
/*  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT   */
/*  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,  */
/*  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT       */
/*  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,  */
/*  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY  */
/*  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT    */
/*  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE  */
/*  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.   */
/*                                                                         */
/* ======================================================================= */

#pragma CODE_SECTION(DSPF_sp_maxloc, ".text:optimized");

#include <DSPF_sp_maxloc.h>
#include "float.h"
#ifdef __TI_COMPILER_VERSION__
#include "c6x.h"
#endif

int DSPF_sp_maxloc(const float* x, int nx)
{
   int    i;
   float  x0, x1, x2, x3, max0, max1, max2, max3;
   int loc_max1,loc_max2,loc_max3,loc_max0;
   __float2_t x_01, x_23;

   max0 = -FLT_MAX;
   max1 = -FLT_MAX;
   max2 = -FLT_MAX;
   max3 = -FLT_MAX;
   loc_max1 = -0xFFFF;
   loc_max2 = -0xFFFF;
   loc_max3 = -0xFFFF;
   loc_max0 = -0xFFFF;
   _nassert(nx % 4 == 0);
   _nassert(nx > 0);
   _nassert((int)x % 8 == 0);

   #pragma MUST_ITERATE(1,,)
   for (i = 0; i < nx; i+=4)
   {
      x_01 = _amem8_f2((void*)&x[i]);
      x_23 = _amem8_f2((void*)&x[i+2]);
      x0   = _hif2(x_01);
      x1   = _lof2(x_01);
      x2   = _hif2(x_23);
      x3   = _lof2(x_23);
      if (x0 > max0)
      {
        max0 = x0;
        loc_max0 = i;
      }
      if (x1 > max1)
      {
        max1 = x1;
        loc_max1 = i+1;
      }
      if (x2 > max2)
      {
          max2 = x2;
          loc_max1 = i+2;
      }
      if (x3 > max3)
      {
        max3 = x3;
        loc_max1 = i+3;
      }
   }
   if (max0 > max1)
   {
     max1 = max0;
     loc_max1 = loc_max0;
   }
   if (max2 > max3)
   {
	 max3 = max2;
	 loc_max3 = loc_max2;
   }

   if (max3 > max1)
     return loc_max3;
   else    
     return loc_max1;
}

/* ======================================================================= */
/*  End of file:  DSPF_sp_maxval.c                                         */
/* ----------------------------------------------------------------------- */
/*            Copyright (c) 2011 Texas Instruments, Incorporated.          */
/*                           All Rights Reserved.                          */
/* ======================================================================= */

