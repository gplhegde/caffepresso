/* ======================================================================== *
 * IMGLIB -- TI Image and Video Processing Library                          *
 *                                                                          *
 *                                                                          *
 * Copyright (C) 2012 Texas Instruments Incorporated - http://www.ti.com/   *
 *                                                                          *
 *                                                                          *
 *  Redistribution and use in source and binary forms, with or without      *
 *  modification, are permitted provided that the following conditions      *
 *  are met:                                                                *
 *                                                                          *
 *    Redistributions of source code must retain the above copyright        *
 *    notice, this list of conditions and the following disclaimer.         *
 *                                                                          *
 *    Redistributions in binary form must reproduce the above copyright     *
 *    notice, this list of conditions and the following disclaimer in the   *
 *    documentation and/or other materials provided with the                *
 *    distribution.                                                         *
 *                                                                          *
 *    Neither the name of Texas Instruments Incorporated nor the names of   *
 *    its contributors may be used to endorse or promote products derived   *
 *    from this software without specific prior written permission.         *
 *                                                                          *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS     *
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT       *
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR   *
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT    *
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,   *
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT        *
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,   *
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY   *
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT     *
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE   *
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.    *
 * ======================================================================== */

/* ======================================================================== */
/*  NAME                                                                    */
/*      IMG_conv_11x11_i16s_c16s                                            */
/*                                                                          */
/*  USAGE                                                                   */
/*      This routine is C-callable and can be called as:                    */
/*                                                                          */
/*          void IMG_conv_11x11_i16s_c16s                                   */
/*          (                                                               */
/*              const short        *restrict   imgin_ptr,                   */
/*                    short        *restrict  imgout_ptr,                   */
/*                    short                        width,                   */
/*                    short                        pitch,                   */
/*              const short        *restrict    mask_ptr,                   */
/*                    short                        shift                    */
/*          )                                                               */
/*                                                                          */
/*      imgin_ptr :   pointer to an input  array of 16 bit pixels           */
/*      imgout_ptr:   pointer to an output array of 16 bit pixels           */
/*      width     :   number of output pixels                               */
/*      pitch     :   number of columns in the image                        */
/*      mask_ptr  :   pointer to 16 bit filter mask.                        */
/*      shift     :   user specified shift amount                           */
/*                                                                          */
/*  DESCRIPTION                                                             */
/*      The convolution kernel accepts eleven rows of 'pitch' input pixels  */
/*      and produces one row of 'width' output pixels using the input       */
/*      mask of 11 by 11. This convolution performs a point by point mult-  */
/*      iplication of 11 by 11 mask with the input image. The result of 121 */
/*      multiplications are then summed together to produce a 40-bit conv-  */
/*      olution intermediate sum. Overflow while accumulation is not hand-  */
/*      led. However assumptions are made on filter gain to avoid overflow. */
/*      The user defined shift value is used to shift this convolution sum  */
/*      down to the short range and store in an output array. The result    */
/*      being stored is also range limited between -32768 to 32767 and will */
/*      be saturated accordingly. The mask is moved one column at a time,   */
/*      advancing the mask over the entire image until the entire 'width'   */
/*      is covered.The input, output image pixels and the masks are         */
/*      provided as 16-bit signed values.                                   */
/*                                                                          */
/*      The natural C implementation has no restrictions. The Optimized     */
/*      and IntrinsiC codes has restrictions as noted in the ASSUMPTIONS    */
/*      below.                                                              */
/*                                                                          */
/*  ASSUMPTIONS                                                             */
/*      1. width:   'width' >=4 and a multiple of 4.                        */
/*      2. pitch:   'pitch' >= 'width'                                      */
/*      3. Input and output arrays should not overlap.                      */
/*      4. Output array should be double word aligned.                      */
/*      5. Input and mask arrays should be half-word aligned.               */
/*      6. Internal accuracy of the computations is 40 bits. To ensure      */
/*         correctness on a 16 bit input data, the maximum permissible      */
/*         filter gain in terms of bits is 24-bits i.e. the cumulative sum  */
/*         of the absolute values of the filter coefficients should not     */
/*         exceed 2^24 - 1.                                                 */
/*                                                                          */
/*  COMPATIBILITY                                                           */
/*      This code is compatible only with C64x+.                            */
/* ======================================================================== */

#pragma CODE_SECTION(IMG_conv_11x11_i16s_c16s,   ".text:optimized");


void IMG_conv_11x11_i16s_c16s
(
    const short   *restrict    imgin_ptr,
          short   *restrict   imgout_ptr,
          short                    width,
          short                    pitch,
    const short   *restrict     mask_ptr,
          short                    shift
)
{
  int             i,              k;  
  int             sum1_int,       sum2_int,           sum3_int;
  int             sum4_int;    

  int             mask01,         mask02,             mask03;
  int             mask04,         mask05,             mask06;  

  int             pix_dc;
  
  __int40_t       sum1,           sum2,               sum3;
  __int40_t       sum4;

  double          y1y0_a,         y3y2_a;

  long long       mask_3210,      mask_7654,          mask_ba98;
  long long       y1y0_10,        y1y0_32,            y1y0_54;
  long long       y1y0_76,        y1y0_98,            y3y2_10;
  long long       y3y2_32,        y3y2_54,            y3y2_76;
  long long       y3y2_98;

  long long       pix_3210,       pix_7654,           pix_ba98;
  long long       pix_5432,       pix_9876,           pix_dcba;

  const short     *in; 

  
  _nassert((int)imgout_ptr % 8 == 0);  /* Output Dword aligned            */
  _nassert((int)(width)    % 4 == 0);  /* output samples multiple of 4    */

  /* -------------------------------------------------------------------- */
  /*  Loop is manually unrolled by 4                                      */
  /* -------------------------------------------------------------------- */

  for (i=0; i<width; i+=4) {
    /* ---------------------------------------------------------------- */
    /* Introduce a pointer to point to the ith input image pixel within */
    /* the iteration. Also zero accumulator values for two outputs      */
    /* ---------------------------------------------------------------- */
    in        =     &imgin_ptr[i];
    sum1      =     0;
    sum2      =     0;
    sum3      =     0;
    sum4      =     0;

    /* ---------------------------------------------------------------- */
    /*  This loop calculates the convolution sum for a single row       */
    /* ---------------------------------------------------------------- */
    for (k=0; k<11; k++) {
      /* ------------------------------------------------------------ */
      /* Load 14 pixels that can be used to calculate four output     */
      /* pixels from the kth row                                      */
      /* ------------------------------------------------------------ */
      pix_3210  = _mem8_const( ( void * ) &in[k * pitch + 0]) ;
      pix_7654  = _mem8_const( ( void * ) &in[k * pitch + 4]);
      pix_ba98  = _mem8_const( ( void * ) &in[k * pitch + 8]);     
      pix_dc    = _mem4_const( ( void * ) &in[k * pitch + 12]); 
  
      pix_5432  = _itoll( _loll(pix_7654), _hill(pix_3210));
      pix_9876  = _itoll( _loll(pix_ba98), _hill(pix_7654));
      pix_dcba  = _itoll( pix_dc, _hill(pix_ba98));
 
      /* ------------------------------------------------------------ */
      /*  Load the 11 mask values to be used in the current row       */
      /* ------------------------------------------------------------ */
      mask_3210 = _mem8_const ((void *) &mask_ptr[(10-k) * 11 + 0]);
      mask_7654 = _mem8_const ((void *) &mask_ptr[(10-k) * 11 + 4]);
      mask_ba98 = _mem8_const ((void *) &mask_ptr[(10-k) * 11 + 8]);

      /* ------------------------------------------------------------ */
      /* Extract two mask values into an int so that it can be used   */
      /* in ddotpl2                                                   */
      /* ------------------------------------------------------------ */
      mask01    = _packhl2(_loll(mask_ba98),_hill(mask_ba98));
      mask02    = _packhl2(_hill(mask_7654),_loll(mask_ba98));
      mask03    = _packhl2(_loll(mask_7654),_hill(mask_7654));
      mask04    = _packhl2(_hill(mask_3210),_loll(mask_7654));
      mask05    = _packhl2(_loll(mask_3210),_hill(mask_3210));
      mask06    = _loll(mask_3210) & 0xffff;
      mask06   |= (mask06 << 16);

     /* ------------------------------------------------------------- */
     /* Calculate the sum of products using ddotpl2.The lower half    */
     /* of the output contains values for the first pixel and the     */
     /* upper half for the 2nd pixel                                  */
     /* ------------------------------------------------------------- */
      y1y0_10   = _ddotpl2(pix_3210, mask01);
      y1y0_32   = _ddotpl2(pix_5432, mask02);
      y1y0_54   = _ddotpl2(pix_7654, mask03);
      y1y0_76   = _ddotpl2(pix_9876, mask04);
      y1y0_98   = _ddotpl2(pix_ba98, mask05);
      y1y0_a    = _mpy2(_hill(pix_ba98), mask06);

      y3y2_10   = _ddotpl2(pix_5432, mask01);
      y3y2_32   = _ddotpl2(pix_7654, mask02);
      y3y2_54   = _ddotpl2(pix_9876, mask03);
      y3y2_76   = _ddotpl2(pix_ba98, mask04);
      y3y2_98   = _ddotpl2(pix_dcba, mask05);
      y3y2_a    = _mpy2(pix_dc, mask06);

     /* ------------------------------------------------------------- */
     /* Calculate sum1,sum2,sum3 and sum4 which holds the convolution */
     /*  sums for four consecutive pixels.                            */
     /* ------------------------------------------------------------- */
      sum1      = _lsadd(_loll(y1y0_10), sum1);
      sum1      = _lsadd(_loll(y1y0_32), sum1);
      sum1      = _lsadd(_loll(y1y0_54), sum1);
      sum1      = _lsadd(_loll(y1y0_76), sum1);
      sum1      = _lsadd(_loll(y1y0_98), sum1);
      sum1      = _lsadd(_lo(y1y0_a),    sum1);

      sum2      = _lsadd(_hill(y1y0_10), sum2);
      sum2      = _lsadd(_hill(y1y0_32), sum2);
      sum2      = _lsadd(_hill(y1y0_54), sum2);
      sum2      = _lsadd(_hill(y1y0_76), sum2);
      sum2      = _lsadd(_hill(y1y0_98), sum2);
      sum2      = _lsadd(_hi(y1y0_a),    sum2);  
      
      sum3      = _lsadd(_loll(y3y2_10), sum3);
      sum3      = _lsadd(_loll(y3y2_32), sum3);
      sum3      = _lsadd(_loll(y3y2_54), sum3);
      sum3      = _lsadd(_loll(y3y2_76), sum3);
      sum3      = _lsadd(_loll(y3y2_98), sum3);
      sum3      = _lsadd(_lo(y3y2_a),    sum3);

      sum4      = _lsadd(_hill(y3y2_10), sum4);
      sum4      = _lsadd(_hill(y3y2_32), sum4);
      sum4      = _lsadd(_hill(y3y2_54), sum4);
      sum4      = _lsadd(_hill(y3y2_76), sum4);
      sum4      = _lsadd(_hill(y3y2_98), sum4);
      sum4      = _lsadd(_hi(y3y2_a),    sum4);
    }
    /* ---------------------------------------------------------------- */
    /* Shift the sums by the user defined value,saturate and store      */
    /* ---------------------------------------------------------------- */
    sum1_int = sum1 >> shift; 
    sum2_int = sum2 >> shift;
    sum3_int = sum3 >> shift; 
    sum4_int = sum4 >> shift;  

    _amemd8((void * )&imgout_ptr[i]) = _itod(_spack2(sum4_int,sum3_int),
                                             _spack2(sum2_int,sum1_int));
  }
}

/* ======================================================================== */
/*  End of file:  IMG_conv_11x11_i16s_c16s.c                                */
/* ======================================================================== */

