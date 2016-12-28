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
/*      IMG_corr_11x11_i16s_c16s                                            */
/*                                                                          */
/*  USAGE                                                                   */
/*      This routine is C-callable and can be called as:                    */
/*                                                                          */
/*          void IMG_corr_11x11_i16s_c16s                                   */
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
/*      imgout_ptr:  pointer to an output array of 16 bit pixels            */
/*      width     :  number of output pixels                                */
/*      pitch     :  number of columns in the input image                   */
/*      mask_ptr  :  pointer to 16 bit filter mask.                         */
/*      shift     :  user specified shift amount                            */
/*      round     :  user specified round value                             */
/*                                                                          */
/*  DESCRIPTION                                                             */
/*      The correlation kernel accepts 11 rows of 'pitch' input pixels and  */
/*      produces one row of 'width' output pixels using the input mask of   */
/*      11x11. This correlation performs a point by point multiplication    */
/*      of 11x11 mask with the input image. The result of the 121 multipli- */
/*      cations are then summed together to produce a 40-bit sum. A         */
/*      rounding constant is added to the sum and then right-shifted to     */
/*      produce a 32-bit value. This is stored in the output array.         */
/*      Overflow and Saturation of the accumulated sum is not handled.      */
/*      However assumptions are made on filter gain to avoid them.          */
/*      The mask is moved one column at a time, advancing the mask over     */
/*      the entire image until the entire 'width' is covered. The masks     */
/*      are provided as 16-bit signed values and the input image pixels     */
/*      are provided as 16-bit  signed values and the output pixels will    */
/*      be 32-bit signed.The mask to be correlated is typically part of the */
/*      input image or another image.                                       */
/*                                                                          */
/*      The natural C implementation has no restrictions. The Optimized     */
/*      and Intrinsic codes has restrictions as noted in the ASSUMPTIONS    */
/*      below.                                                              */
/*                                                                          */
/*  ASSUMPTIONS                                                             */
/*      1. width:   'width' >=2 and a multiple of 2.                        */
/*      2. pitch:   'pitch' >= 'width'                                      */
/*      3. Input and output arrays should not overlap                       */
/*      4. Output array should be double word aligned                       */
/*      5. Input and mask arrays should be half-word aligned                */
/*      6. Internal accuracy of the computations is 40 bits.                */
/*      7. Shift is appropriate to produce a 32-bit result. Overflows are   */
/*         not handled.                                                     */
/*      8. Range of filter co-efficients is -32767 to 32767.                */
/*                                                                          */
/* ======================================================================== */

#pragma CODE_SECTION(IMG_corr_11x11_i16s_c16s_short,   ".text:optimized");

void IMG_corr_11x11_i16s_c16s_short
(
    const  short    *restrict  imgin_ptr,
           short      *restrict imgout_ptr,
           short                   width,
           short                   pitch,
    const  short    *restrict   mask_ptr,
           short                   shift
)
{
  int            i,               k;
  int            row1,            row2;
  int            mask01,          mask02,          mask03;
  int            mask04,          mask05,          mask06;

  __int40_t      sum1,            sum2;

  double         mask_row_00,     mask_row_01,     mask_row_02;

  long long      temp1,           temp2,           temp3;
  long long      temp4,           temp5;
  long long      row_00,          row_01,          row_02;
  long long      row_5432,        row_9876;

  const short    *in;
  const short    *maskptr;

  _nassert((int) imgout_ptr % 4 == 0);  /* Output word aligned         */
  _nassert((int) width      % 2 == 0);  /* output samples multiple of 2 */

  for (i=0; i<width; i+=2) {
    in      = &imgin_ptr[i];
    maskptr = &mask_ptr[0];
    sum1    = 0;
    sum2    = 0;

    /* ---------------------------------------------------------------- */
    /*  This loop calculates the convolution sum for a single row       */
    /* ---------------------------------------------------------------- */

    for (k=0; k<11; k++) {
      /* ------------------------------------------------------------ */
      /* Load 12 pixels that can be used to calculate 2 output pixels */
      /* ------------------------------------------------------------ */
      row_00   = _mem8_const( ( void * ) &in[k * pitch    ]) ;
      row_01   = _mem8_const( ( void * ) &in[k * pitch + 4]) ;
      row_02   = _mem8_const( ( void * ) &in[k * pitch + 8]) ;
      row1     = _hill(row_02) & 0xffff    ;
      row2     = (_hill(row_02 >> 16)) & 0xffff;
      row_5432 = _itoll( _loll(row_01), _hill(row_00));
      row_9876 = _itoll( _loll(row_02), _hill(row_01));

      /* ------------------------------------------------------------ */
      /*  Load the 11 mask values to be used in the current row       */
      /* ------------------------------------------------------------ */
      mask_row_00 = _memd8_const( ( void * ) &maskptr[k * 11    ]);
      mask_row_01 = _memd8_const( ( void * ) &maskptr[k * 11 + 4]);
      mask_row_02 = _memd8_const( ( void * ) &maskptr[k * 11 + 8]);

      /* ------------------------------------------------------------ */
      /* Extract 2 mask values to an int so it can be used in ddotpl2 */
      /* ------------------------------------------------------------ */
      mask01 = _lo(mask_row_00);
      mask02 = _hi(mask_row_00);
      mask03 = _lo(mask_row_01);
      mask04 = _hi(mask_row_01);
      mask05 = _lo(mask_row_02);
      mask06 = _hi(mask_row_02) & 0xffff;

      /* ------------------------------------------------------------ */
      /* Calculate the sum of products using ddotpl2.The lower half of*/
      /* thr output contains values for the first pixel and the upper */
      /* half for the 2nd pixel                                       */
      /* ------------------------------------------------------------ */
      temp1 = _ddotpl2(row_00, mask01);
      temp2 = _ddotpl2(row_5432, mask02);
      temp3 = _ddotpl2(row_01, mask03);
      temp4 = _ddotpl2(row_9876, mask04);
      temp5 = _ddotpl2(row_02, mask05);


      /* ------------------------------------------------------------ */
      /* Calculate sum1,sum2 which has convolution sums for 2 pixels  */
      /* ------------------------------------------------------------ */
      sum1 = _lsadd(_loll(temp1), sum1);
      sum1 = _lsadd(_loll(temp2), sum1);
      sum1 = _lsadd(_loll(temp3), sum1);
      sum1 = _lsadd(_loll(temp4), sum1);
      sum1 = _lsadd(_loll(temp5), sum1);
      sum1 = _lsadd(_dotp2(row1,mask06), sum1);

      sum2 = _lsadd(_hill(temp1),sum2);
      sum2 = _lsadd(_hill(temp2),sum2);
      sum2 = _lsadd(_hill(temp3),sum2);
      sum2 = _lsadd(_hill(temp4),sum2);
      sum2 = _lsadd(_hill(temp5),sum2);
      sum2 = _lsadd(_dotp2(row2,mask06),sum2);
    }

    /* ---------------------------------------------------------------- */
    /*  Shift the sum  by 7 bits to avoid overflow                      */
    /* ---------------------------------------------------------------- */
    sum1   = (sum1 >> shift);
    sum2   = (sum2 >> shift);

    _amem4( ( void * )&imgout_ptr[i] ) = _spack2(sum2, sum1);
  }
}

/* ======================================================================== */
/*  End of file:  IMG_corr_11x11_i16s_c16s.c                                */
/* ======================================================================== */

