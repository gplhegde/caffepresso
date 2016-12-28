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
/*      IMG_corr_3x3_i16s_c16s                                              */
/*                                                                          */
/*  USAGE                                                                   */
/*      This routine is C-callable and can be called as:                    */
/*                                                                          */
/*          void IMG_corr_3x3_i16s_c16s                                     */
/*          (                                                               */
/*              const short      *restrict  imgin_ptr,                      */
/*                    short        *restrict imgout_ptr,                      */
/*                    short                     width,                      */
/*                    short                     pitch,                      */
/*              const short      *restrict   mask_ptr,                      */
/*                    short                     shift,                      */
/*          )                                                               */
/*                                                                          */
/*      imgin_ptr :  pointer to an input  array of 16 bit pixels            */
/*      imgout_ptr:  pointer to an output array of 32 bit pixels            */
/*      width     :  number of output pixels                                */
/*      pitch     :  number of columns in the input image                   */
/*      mask_ptr  :  pointer to 16 bit filter mask.                         */
/*      shift     :  user specified shift amount                            */
/*      round     :  user specified round value                             */
/*                                                                          */
/*  DESCRIPTION                                                             */
/*      The correlation kernel accepts 3 rows of 'pitch' input pixels and   */
/*      produces one row of 'width' output pixels using the input mask of   */
/*      3x3. This correlation performs a point by point multiplications     */
/*      of 3x3 mask with the input image. The result of the 9 multiplic-    */
/*      ations are then summed together to produce a 40-bit sum which is    */
/*      added to user specified round value and then stored in an output    */
/*      array. Overflow and Saturation of the accumulated sum is not han-   */
/*      dled. However assumptions are made on filter gain to avoid them.    */
/*      The mask is moved one column at a time, advancing the mask over     */
/*      the entire image until the entire 'width' is covered. The masks     */
/*      are provided as 16-bit signed values and the input image pixels     */
/*      are provided as 16-bit  signed values and the output pixels will    */
/*      be 32-bit signed.The image mask to be correlated is typically part  */
/*      of the input image or another image.                                */
/*                                                                          */
/*      The natural C implementation has no restrictions. The Optimized     */
/*      and Intrinsic codes has restrictions as noted in the ASSUMPTIONS    */
/*      below.                                                              */
/*                                                                          */
/*  ASSUMPTIONS                                                             */
/*      1. width:   'width' >=4 and a multiple of 4.                        */
/*      2. pitch:   'pitch' >= 'width'                                      */
/*      3. Input and Output arrays should not overlap                       */
/*      4. Output array should be double word aligned.                      */
/*      5. Input and mask arrays should be half-word aligned.               */
/*      6. Internal accuracy of the computations is 40 bits. To ensure      */
/*         correctness on a 16 bit input data, the maximum permissible      */
/*         filter gain in terms of bits is 24-bits i.e. the cumulative sum  */
/*         of the absolute values of the filter coefficients should not     */
/*         exceed 2^24 - 1.                                                 */
/*      7. Shift is appropriate to produce a 32-bit result.                 */
/*      8. Range of filter co-efficients is -32767 to 32767.                */
/*                                                                          */
/* ======================================================================== */

#pragma CODE_SECTION(IMG_corr_3x3_i16s_c16s_short,   ".text:optimized");

void IMG_corr_3x3_i16s_c16s_short
(
    const  short    *restrict  imgin_ptr,
           short      *restrict imgout_ptr,
           short                   width,
           short                   pitch,
    const  short    *restrict   mask_ptr,
           short                   shift
)
{
  int              sum01_0,            sum01_1;
  int              sum01_2,            sum01_3;
  __int40_t        sum0,               sum1;
  __int40_t        sum2,               sum3;
  int              mask88,             mask52,         mask4_3;
  int              mask2,              mask5,          mask8;

  long long        sum0_01,            sum1_01,        sum2_01;
  long long        sum0_23,            sum1_23,        sum2_23;
  long long        pix0_3210,          pix1_3210,      pix2_3210;
  long long        pix0_5432,          pix1_5432,      pix2_5432;
  long long        pix13_03_12_02,     pix15_05_14_04;

  unsigned int     i;

  double           mask7_4,            mask3_0;
  double           sum2_0,             sum2_1;

  /* ---------------------------------------------------------------- */
  /*                    load the mask values                          */
  /* ---------------------------------------------------------------- */
  mask3_0  = _memd8_const( ( void * ) &mask_ptr[0] );
  mask7_4  = _memd8_const( ( void * ) &mask_ptr[4] );

  /* ---------------------------------------------------------------- */
  /* The last mask values of each row are loaded into an int          */
  /* ---------------------------------------------------------------- */
  mask2    = _extu(_hi(mask3_0) , 16 , 16);
  mask5    = _extu(_lo(mask7_4) ,  0 , 16);
  mask8    = mask_ptr[8];

  mask4_3  = _packlh2(_lo(mask7_4) , _hi(mask3_0));
  mask88   = _pack2(mask8 , mask8);
  mask52   = _pack2(mask5 , mask2);

  /* ---------------------------------------------------------------- */
  /* Inform the compiler about the alignments                         */
  /* ---------------------------------------------------------------- */
  _nassert((int)imgout_ptr % 8 == 0);  /* Output Dword aligned        */
  _nassert((int)(width)    % 4 == 0);  /* output samples multiple of 4*/

  for (i=0; i<width; i+=4) {
    sum0    =   0;
    sum1    =   0;
    sum2    =   0;
    sum3    =   0;

    /* ---------------------------------------------------------------- */
    /*  Load 6 pixels at a time from each of the 3 rows using double    */
    /*  word memory accesses. 2 pixels are redundant in each row.       */
    /* ---------------------------------------------------------------- */
    pix0_3210   = _mem8_const((void *) &imgin_ptr[i]);
    pix1_3210   = _mem8_const((void *) &imgin_ptr[i + pitch]);
    pix2_3210   = _mem8_const((void *) &imgin_ptr[i + 2 * pitch]);

    pix0_5432   = _mem8_const((void *) &imgin_ptr[i + 2]);
    pix1_5432   = _mem8_const((void *) &imgin_ptr[i + 2 + pitch]);
    pix2_5432   = _mem8_const((void *) &imgin_ptr[i + 2 + 2 * pitch]);

    /* ---------------------------------------------------------------- */
    /* Packs the pixels from 1st and 2nd rows which are at same column  */
    /* positions (cols 2&3) into the same words, respectively. Here,    */
    /* _hill(pix13_03_12_02)= pix13_03 & _loll(pix13_03_12_02)= pix12_02*/
    /* where pix13_03 contains pixels at 3rd cols of 0th and 1st rows.  */
    /* ---------------------------------------------------------------- */
    pix13_03_12_02 = _dpack2(_hill(pix1_3210), _hill(pix0_3210));
    pix15_05_14_04 = _dpack2(_hill(pix1_5432), _hill(pix0_5432));

    /* ---------------------------------------------------------------- */
    /* Performs double dot-product for rows 0 & 1. The dot-product      */
    /*  obtained at the lower word corresponds to output pixel0, and    */
    /*  the one at the upper word corresponds to output pixel1.         */
    /* ---------------------------------------------------------------- */
    sum0_01  =  _ddotpl2(pix0_3210, _lo(mask3_0));
    sum1_01  =  _ddotpl2(pix1_3210, mask4_3);

    sum0    +=  ((int)_loll(sum0_01));
    sum0    +=  ((int)_loll(sum1_01));
    sum1    +=  ((int)_hill(sum0_01));
    sum1    +=  ((int)_hill(sum1_01));

    sum0_23  =  _ddotpl2(pix0_5432, _lo(mask3_0));
    sum1_23  =  _ddotpl2(pix1_5432, mask4_3);

    sum2    +=  ((int)_loll(sum0_23));
    sum2    +=  ((int)_loll(sum1_23));
    sum3    +=  ((int)_hill(sum0_23));
    sum3    +=  ((int)_hill(sum1_23));

    sum01_0  =  _dotp2(_loll(pix13_03_12_02), mask52);
    sum01_1  =  _dotp2(_hill(pix13_03_12_02), mask52);

    sum0    +=  sum01_0;
    sum1    +=  sum01_1;

    sum01_2  =  _dotp2(_loll(pix15_05_14_04), mask52);
    sum01_3  =  _dotp2(_hill(pix15_05_14_04), mask52);
    sum2    +=  sum01_2;
    sum3    +=  sum01_3;

    sum2_01  =  _ddotpl2(pix2_3210, _hi(mask7_4));
    sum2_0   =  _mpy2(_hill(pix2_3210), mask88);

    sum0    +=  ((int)_loll(sum2_01));
    sum0    +=  ((int)_lo(sum2_0));
    sum1    +=  ((int)_hill(sum2_01));
    sum1    +=  ((int)_hi(sum2_0));

    sum2_23  =  _ddotpl2(pix2_5432, _hi(mask7_4));
    sum2_1   =  _mpy2(_hill(pix2_5432), mask88);

    sum2    +=  ((int)_loll(sum2_23));
    sum2    +=  ((int)_lo(sum2_1));
    sum3    +=  ((int)_hill(sum2_23));
    sum3    +=  ((int)_hi(sum2_1));

    /* ---------------------------------------------------------------- */
    /*  Performs right shift to handle overflow.sum0, sum1, sum2        */
    /*  &sum3 corresponds to the outputs for 4 adjacent pixels  with    */
    /*  ref to the current postion of output pointer given by offset i. */
    /* ---------------------------------------------------------------- */
    sum0     = (sum0 >>  shift);
    sum1     = (sum1 >>  shift);
    sum2     = (sum2 >>  shift);
    sum3     = (sum3 >>  shift);

    _amem4( ( void * )&imgout_ptr[i] ) = _spack2(sum1, sum0);
    _amem4( ( void * )&imgout_ptr[i + 2] ) = _spack2(sum3, sum2);
  }
}

/* ======================================================================== */
/*   End of file:  IMG_corr_3x3_i16s_c16s.c                                 */
/* ======================================================================== */

