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
/*      IMG_conv_5x5_i16s_c16s                                              */
/*                                                                          */
/*  USAGE                                                                   */
/*      This routine is C-callable and can be called as:                    */
/*                                                                          */
/*          void IMG_conv_5x5_i16s_c16s                                     */
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
/*      The convolution kernel accepts five rows of 'pitch' input pixels    */
/*      and produces one row of 'width' output pixels using the input       */
/*      mask of 5 by 5. This convolution performs a point by point mult-    */
/*      iplication of 5 by 5 mask with the input image. The result of 25    */
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
/*      1. width:   'width' >=2 and a multiple of 2.                        */
/*      2. pitch:   'pitch' >= 'width'.                                     */
/*      3. Input and output arrays should not overlap                       */
/*      4. Output array should be word aligned                              */
/*      5. The mask and input arrays should be half-word aligned.           */
/*      6. Internal accuracy of the computations is 32 bits. To ensure      */
/*         correctness on a 16 bit input data, the maximum permissible      */
/*         filter gain in terms of bits is 16-bits i.e. the cumulative sum  */
/*         of the absolute values of the filter coefficients should not     */
/*         exceed 2^16 - 1.                                                 */
/*                                                                          */
/*  COMPATIBILITY                                                           */
/*      This code is compatible only with C66x.                             */
/*                                                                          */
/* ======================================================================== */

#pragma CODE_SECTION(IMG_conv_5x5_i16s_c16s,   ".text:optimized");


void IMG_conv_5x5_i16s_c16s
(
    const short   *restrict    imgin_ptr,
          short   *restrict   imgout_ptr,
          short                    width,
          short                    pitch,
    const short   *restrict     mask_ptr,
          short                    shift
)
{
  /* ---------------------------------------------------------------- */
  /* in0 to in4 contains pointers to lines 1 to 5                     */
  /* ---------------------------------------------------------------- */
  int                     i,              count;
  int                     sum0,           sum1;

  double                  dm1;

  long long               dp1,            dp2;
  long long               row1_3210,      row2_3210,      row3_3210;
  long long               row4_3210,      row5_3210,      row1_5432; 
  long long               row2_5432,      row3_5432,      row4_5432;
  long long               row5_5432;
  long long               mask1_3210,     mask2_3210,     mask3_3210;
  long long               mask4_3210,     mask5_3210,     mask_temp;
   
  unsigned int            row1_54,        row2_54,        row3_54;
  unsigned int            row4_54,        row5_54;     

  unsigned int            mask1_44,       mask2_44,       mask3_44;
  unsigned int            mask4_44,       mask5_44;

  const short *restrict   in0;
  const short *restrict   in1;
  const short *restrict   in2;
  const short *restrict   in3;
  const short *restrict   in4;

  /* -------------------------------------------------------------------- */
  /*  Load mask values (reverse order for mask rotation)                  */
  /* -------------------------------------------------------------------- */
  mask_temp  = _mem8_const ((void *) &mask_ptr[21]);
  mask1_3210 = _itoll(_packlh2(_loll(mask_temp),_loll(mask_temp)),
                      _packlh2(_hill(mask_temp),_hill(mask_temp)));
  mask1_44   = _pack2((int) mask_ptr[20],  (int) mask_ptr[20]);

  mask_temp  = _mem8_const ((void *) &mask_ptr[16]);
  mask2_3210 = _itoll(_packlh2(_loll(mask_temp),_loll(mask_temp)),
                      _packlh2(_hill(mask_temp),_hill(mask_temp)));
  mask2_44   = _pack2((int) mask_ptr[15],  (int) mask_ptr[15]);

  mask_temp  = _mem8_const ((void *) &mask_ptr[11]);
  mask3_3210 = _itoll(_packlh2(_loll(mask_temp),_loll(mask_temp)),
                      _packlh2(_hill(mask_temp),_hill(mask_temp)));
  mask3_44   = _pack2((int) mask_ptr[10], (int) mask_ptr[10]);

  mask_temp  = _mem8_const ((void *) &mask_ptr[6]);
  mask4_3210 = _itoll(_packlh2(_loll(mask_temp),_loll(mask_temp)),
                      _packlh2(_hill(mask_temp),_hill(mask_temp)));
  mask4_44   = _pack2((int) mask_ptr[5], (int) mask_ptr[5]);
 
  mask_temp  = _mem8_const ((void *) &mask_ptr[1]);
  mask5_3210 = _itoll(_packlh2(_loll(mask_temp),_loll(mask_temp)),
                      _packlh2(_hill(mask_temp),_hill(mask_temp)));
  mask5_44   = _pack2((int) mask_ptr[0], (int) mask_ptr[0]);
  
  /* -------------------------------------------------------------------- */
  /* Initialise pointers corresponding to each row                        */
  /* -------------------------------------------------------------------- */
  in0 = imgin_ptr; 
  in1 = in0 + pitch; 
  in2 = in1 + pitch;
  in3 = in2 + pitch;  
  in4 = in3 + pitch;

  /* -------------------------------------------------------------------- */
  /* Load the number of times loop to be executed into count              */
  /* -------------------------------------------------------------------- */
  count = width >> 1;
      
  _nassert((int)imgout_ptr % 4 == 0);  /* Output word aligned             */
  _nassert((int)(width)    % 2 == 0);  /* output samples multiple of 2    */
  _nassert((int)(pitch)    % 2 == 0);  /* # of coloumns multiple of 2     */

  /* -------------------------------------------------------------------- */
  /* Two output pixels are calculated in each iteration                   */
  /* -------------------------------------------------------------------- */

  for (i=0; i<count; i++) {
    /* ---------------------------------------------------------------- */
    /* Zero two accumulators to store two output samples per iteration  */
    /* ---------------------------------------------------------------- */
    sum0      = 0;
    sum1      = 0;

    /* ---------------------------------------------------------------- */
    /* Load pixel values for row 1,calculate partial sum of products    */
    /* ---------------------------------------------------------------- */
    row1_3210 = _mem8_const(in0);
    row1_54   = _mem4_const(in0 + 4);
    row1_5432 = _itoll(row1_54, _hill(row1_3210));

    dp1       = _ddotpl2(row1_3210, _loll(mask1_3210));
    dp2       = _ddotpl2(row1_5432, _hill(mask1_3210));
    dm1       = _mpy2(row1_54, mask1_44);

    sum0      = _loll(dp1) + _loll(dp2) + _lo(dm1);
    sum1      = _hill(dp1) + _hill(dp2) + _hi(dm1);
    in0       = in0 + 2;

    /* ---------------------------------------------------------------- */
    /* Load pixel values for row 2,calculate partial sum of products    */
    /* ---------------------------------------------------------------- */
    row2_3210 = _mem8_const(in1);
    row2_54   = _mem4_const(in1 + 4);
    row2_5432 = _itoll(row2_54, _hill(row2_3210));

    dp1       = _ddotpl2(row2_3210, _loll(mask2_3210));
    dp2       = _ddotpl2(row2_5432, _hill(mask2_3210));
    dm1       = _mpy2(row2_54, mask2_44);

    sum0     += _loll(dp1) + _loll(dp2) + _lo(dm1);
    sum1     += _hill(dp1) + _hill(dp2) + _hi(dm1);
    in1       = in1 + 2;

    /* ---------------------------------------------------------------- */
    /* Load pixel values for row 3,calculate partial sum of products    */
    /* ---------------------------------------------------------------- */
    row3_3210 = _mem8_const(in2);
    row3_54   = _mem4_const(in2 + 4);
    row3_5432 = _itoll(row3_54, _hill(row3_3210));

    dp1       = _ddotpl2(row3_3210, _loll(mask3_3210));
    dp2       = _ddotpl2(row3_5432, _hill(mask3_3210));
    dm1       = _mpy2(row3_54, mask3_44);

    sum0     += _loll(dp1) + _loll(dp2) + _lo(dm1);
    sum1     += _hill(dp1) + _hill(dp2) + _hi(dm1);
    in2       = in2 + 2;

    /* ---------------------------------------------------------------- */
    /* Load pixel values for row 4,calculate partial sum of products    */
    /* ---------------------------------------------------------------- */
    row4_3210 = _mem8_const(in3);
    row4_54   = _mem4_const(in3 + 4);
    row4_5432 = _itoll(row4_54, _hill(row4_3210));

    dp1       = _ddotpl2(row4_3210, _loll (mask4_3210));
    dp2       = _ddotpl2(row4_5432, _hill(mask4_3210));
    dm1       = _mpy2(row4_54, mask4_44);

    sum0     += _loll(dp1) + _loll(dp2) + _lo(dm1);
    sum1     += _hill(dp1) + _hill(dp2) + _hi(dm1);
    in3       = in3 + 2;
    
    /* ---------------------------------------------------------------- */
    /* Load pixel values for row 5,calculate partial sum of products    */
    /* ---------------------------------------------------------------- */
    row5_3210 = _mem8_const(in4);
    row5_54   = _mem4_const(in4 + 4);
    row5_5432 = _itoll(row5_54, _hill(row5_3210));

    dp1       = _ddotpl2(row5_3210, _loll(mask5_3210));
    dp2       = _ddotpl2(row5_5432, _hill(mask5_3210));

    dm1       = _mpy2(row5_54, mask5_44);
    sum0     += _loll(dp1) + _loll(dp2) + _lo(dm1);
    sum1     += _hill(dp1) + _hill(dp2) + _hi(dm1);
    in4       = in4 + 2;
      
    /* ---------------------------------------------------------------- */
    /* Sum shifted by the user specified value,saturated & stored       */
    /* ---------------------------------------------------------------- */
    sum0    >>= shift; 
    sum1    >>= shift; 
 
    _amem4 (&imgout_ptr[2*i]) = _spack2(sum1, sum0);
  }   
}

/* ======================================================================== */
/*  End of file:  IMG_conv_5x5_i16s_c16s.c                                  */
/* ======================================================================== */
