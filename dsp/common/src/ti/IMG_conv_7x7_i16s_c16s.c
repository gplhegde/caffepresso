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
/*      IMG_conv_7x7_i16s_c16s                                              */
/*                                                                          */
/*  USAGE                                                                   */
/*      This routine is C-callable and can be called as:                    */
/*                                                                          */
/*          void IMG_conv_7x7_i16s_c16s                                     */
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
/*      width     :   # of output pixels                                    */
/*      pitch     :   # of columns in the image                             */
/*      mask_ptr  :   pointer to 16 bit filter mask.                        */
/*      shift     :   user specified shift amount                           */
/*                                                                          */
/*  DESCRIPTION                                                             */
/*      The convolution kernel accepts seven rows of 'pitch' input pixels   */
/*      and produces one row of 'width' output pixels using the input       */
/*      mask of 7 by 7. This convolution performs a point by point mult-    */
/*      iplication of 7 by 7 mask with the input image. The result of 49    */
/*      multiplications are then summed together to produce a 32-bit conv-  */
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
/*      1. width:   'width' >=8 and a multiple of 8.                        */
/*      2. pitch:   'pitch' >= 'width' and a multiple of 4.                 */
/*      3. Input and output arrays should not overlap.                      */
/*      4. Input and output arrays must be double word aligned.             */
/*      5. Mask pointer should be half-word aligned.                        */
/*      6. Internal accuracy of the computations is 32 bits. To ensure      */
/*         correctness on a 16 bit input data, the maximum permissible      */
/*         filter gain in terms of bits is 16-bits i.e. the cumulative sum  */
/*         of the absolute values of the filter coefficients should not     */
/*         exceed 2^16 - 1.                                                 */
/*                                                                          */
/*  COMPATIBILITY                                                           */
/*      This code is compatible only with C64x+.                            */
/* ======================================================================== */

#pragma CODE_SECTION(IMG_conv_7x7_i16s_c16s,   ".text:optimized");


void IMG_conv_7x7_i16s_c16s
(
    const short   *restrict    imgin_ptr,
          short   *restrict   imgout_ptr,
          short                    width,
          short                    pitch,
    const short   *restrict     mask_ptr,
          short                    shift
)
{
  int             i,            k;  
  int             sum1,         sum2,         sum3;
  int             sum4,         sum5,         sum6;
  int             sum7,         sum8;    
  int             row_03;
  
  long long       temp1,        temp2,        temp3;
  long long       row_00,       row_01,       row_02;
  long long       row_5432,     row_9876,     row_dcba;
  long long       temp4;

  long long       mask_row_00,  mask_row_01,  mask_tmp;

  unsigned int    mask77;
  
  const short     *in;


  _nassert((int) imgout_ptr % 8 == 0);  /* Output Double-word aligned */
  _nassert((int) imgin_ptr  % 8 == 0);  /* Input  Double-word aligned */
  _nassert((int) width      % 8 == 0);  /* output samples mult. of 8  */



  for (i=0; i<width; i+=8) {
    /* ---------------------------------------------------------------- */
    /* Introduce a pointer to point to the ith input image pixel within */
    /* the iteration.                                                   */
    /* ---------------------------------------------------------------- */
    in   = &imgin_ptr[i];

    /* ---------------------------------------------------------------- */
    /* Reset the accumulators for eight output samples, on the first    */
    /* iteration of the loop.                                           */
    /* ---------------------------------------------------------------- */
    sum1 = 0;
    sum2 = 0;
    sum3 = 0;
    sum4 = 0;
    sum5 = 0;
    sum6 = 0;
    sum7 = 0;
    sum8 = 0;
 
    /* ---------------------------------------------------------------- */
    /* Initialize the counter to perform 56 multiplications in one row  */
    /* so as to get eight output samples                                */
    /* And inform the compiler that this loop will execute at least 7   */
    /* and multiples of 7 from then.                                    */
    /* ---------------------------------------------------------------- */
    #pragma MUST_ITERATE(7,7,7);
    for (k=0; k<7; k++) {
      /* ------------------------------------------------------------ */
      /* Load 14 input image short values in kth single row           */
      /* ------------------------------------------------------------ */
      row_00      = _amem8_const((void *) &in[k * pitch  + 0]);
      row_01      = _amem8_const((void *) &in[k * pitch  + 4]);
      row_02      = _amem8_const((void *) &in[k * pitch  + 8]);
      row_03      = _amem4_const((void *) &in[k * pitch  + 12]);

      /* ------------------------------------------------------------ */
      /* Load the (6-k)th row of input filter mask short values       */
      /* ------------------------------------------------------------ */
      mask_row_00 = _mem8_const((void *) &mask_ptr[(6-k) * 7 + 0]);
      mask_row_01 = _mem8_const((void *) &mask_ptr[(6-k) * 7 + 4]);
      
      /* ------------------------------------------------------------ */
      /* Reverse row as part of mask rotation                         */
      /* ------------------------------------------------------------ */
      mask_tmp    = mask_row_01;
      mask_row_01 = _itoll(_loll(mask_row_00),_packhl2(_loll(mask_row_00),
                           _hill(mask_row_00)));
      mask_row_00 = _itoll(_packhl2(_hill(mask_row_00),_loll(mask_tmp)),
                           _packhl2(_loll(mask_tmp)   ,_hill(mask_tmp)));

      /* ------------------------------------------------------------ */
      /* Pack the 7th column mask into a integer to ease calculation  */
      /* ------------------------------------------------------------ */
      mask77      = _pack2(_hill(mask_row_01),_hill(mask_row_01));

      /* ------------------------------------------------------------ */
      /* Perform the partial multiplications for first two output     */
      /* samples                                                      */
      /* ------------------------------------------------------------ */
      row_5432    = _itoll(_loll(row_01),_hill(row_00));

      temp1       = _ddotpl2(row_00,    _loll(mask_row_00));
      temp2       = _ddotpl2(row_5432 , _hill(mask_row_00));
      temp3       = _ddotpl2(row_01,    _loll(mask_row_01));
      temp4       = _dtoll(_mpy2(_hill(row_01),mask77));
 
      /* ------------------------------------------------------------ */
      /* Perform the partial convolution sum for first two output     */
      /* samples                                                      */
      /* ------------------------------------------------------------ */
      sum1 += _loll(temp1) + _loll(temp2) + _loll(temp3) + _loll(temp4);
      sum2 += _hill(temp1) + _hill(temp2) + _hill(temp3) + _hill(temp4);
  
      /* ------------------------------------------------------------ */
      /* Perform the partial multiplications for third and fourth     */
      /* output samples                                               */
      /* ------------------------------------------------------------ */
      row_9876    = _itoll(_loll(row_02),_hill(row_01));

      temp1       = _ddotpl2(row_5432, _loll(mask_row_00));
      temp2       = _ddotpl2(row_01,   _hill(mask_row_00));
      temp3       = _ddotpl2(row_9876, _loll(mask_row_01));
      temp4       = _dtoll(_mpy2(_loll(row_02), mask77));
 
      /* ------------------------------------------------------------ */
      /* Perform the partial convolution sum for third and fourth     */
      /* output samples                                               */
      /* ------------------------------------------------------------ */
      sum3 += _loll(temp1) + _loll(temp2) + _loll(temp3) + _loll(temp4);
      sum4 += _hill(temp1) + _hill(temp2) + _hill(temp3) + _hill(temp4);
                
      /* ------------------------------------------------------------ */
      /* Perform the partial multiplications for fifth and sixth      */
      /* output samples                                               */
      /* ------------------------------------------------------------ */
      temp1       = _ddotpl2(row_01,   _loll(mask_row_00));
      temp2       = _ddotpl2(row_9876, _hill(mask_row_00));
      temp3       = _ddotpl2(row_02,   _loll(mask_row_01));
      temp4       = _dtoll(_mpy2(_hill(row_02), mask77)); 

      /* ------------------------------------------------------------ */
      /* Perform the partial convolution sum for fifth and sixth      */
      /* output samples                                               */
      /* ------------------------------------------------------------ */
      sum5 += _loll(temp1) + _loll(temp2) + _loll(temp3) + _loll(temp4);
      sum6 += _hill(temp1) + _hill(temp2) + _hill(temp3) + _hill(temp4);

      /* ------------------------------------------------------------ */
      /* Perform the partial multiplications for seventh and eight    */
      /* output samples                                               */
      /* ------------------------------------------------------------ */
      row_dcba    = _itoll( (row_03), _hill(row_02)); 
                              
      temp1       = _ddotpl2(row_9876, _loll(mask_row_00));
      temp2       = _ddotpl2(row_02,   _hill(mask_row_00));
      temp3       = _ddotpl2(row_dcba, _loll(mask_row_01));
      temp4       = _dtoll(_mpy2((row_03), mask77));
 
      /* ------------------------------------------------------------ */
      /* Perform the partial convolution sum for seventh and eight    */
      /* output samples                                               */
      /* ------------------------------------------------------------ */
      sum7 += _loll(temp1) + _loll(temp2) + _loll(temp3) + _loll(temp4);
      sum8 += _hill(temp1) + _hill(temp2) + _hill(temp3) + _hill(temp4);
    }

    /*----------------------------------------------------------------- */
    /* Shift the int sum by user-defined value to fall in short range   */
    /* also pack four integers to store a double value using amemd      */
    /*----------------------------------------------------------------- */
    sum1 = (int) (sum1 >> shift);
    sum2 = (int) (sum2 >> shift);
    sum3 = (int) (sum3 >> shift);
    sum4 = (int) (sum4 >> shift);
    _amem8((void *) &imgout_ptr[i + 0]) = _itoll(_spack2(sum4,sum3),
                                                 _spack2(sum2,sum1));

    sum5 = (int) (sum5 >> shift);
    sum6 = (int) (sum6 >> shift);
    sum7 = (int) (sum7 >> shift);
    sum8 = (int) (sum8 >> shift);
    _amem8((void *) &imgout_ptr[i + 4]) = _itoll(_spack2(sum8,sum7),
                                                 _spack2(sum6,sum5));
  }
}

/* ======================================================================= */
/*  End of file:  img_corr_7x7_i16s_c16s.c                                 */
/* ======================================================================= */

