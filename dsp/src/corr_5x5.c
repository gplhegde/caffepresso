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
/*      IMG_corr_5x5_i16s_c16s                                              */
/*                                                                          */
/*                                                                          */
/*  USAGE                                                                   */
/*      This routine is C-callable and can be called as:                    */
/*                                                                          */
/*          void IMG_corr_5x5_i16s_c16s                                     */
/*          (                                                               */
/*              const short      *restrict  imgin_ptr,                      */
/*                    short        *restrict imgout_ptr,                      */
/*                    short                     width,                      */
/*                    short                     pitch,                      */
/*              const short      *restrict   mask_ptr,                      */
/*                    short                     shift,                      */
/*                    int                       round                       */
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
/*      The correlation kernel accepts 5 rows of 'pitch' input pixels and   */
/*      produces one row of 'width' output pixels using the input mask of   */
/*      5x5. This correlation performs a point by point multiplications     */
/*      of 5x5 mask with the input image. The result of the 25 multiplic-   */
/*      ations are then summed together to produce a 40-bit sum. A rounding */
/*      constant is added and the resulting value is shifted and stored in  */
/*      an array. Overflow and Saturation of the accumulated sum is not     */
/*      handled. However assumptions are made on filter gain to avoid them. */
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
/*      3. Input and Output arrays should not overlap                       */
/*      4. Output array must be double word aligned                         */
/*      5. Mask and Input array should be half-word aligned.                */
/*      6. Internal accuracy of the computations is 40 bits. To ensure      */
/*         correctness on a 16 bit input data, the maximum permissible      */
/*         filter gain in terms of bits is 24-bits i.e. the cumulative sum  */
/*         of the absolute values of the filter coefficients should not     */
/*         exceed 2^24 - 1.                                                 */
/*      7. Shift is appropriate to produce a 32-bit result.                 */
/*      8. Range of filter co-efficients is -32767 to 32767.                */
/*                                                                          */
/* ======================================================================== */

#pragma CODE_SECTION(IMG_corr_5x5_i16s_c16s_short,   ".text:optimized");

void IMG_corr_5x5_i16s_c16s_short
(
    const  short    *restrict  imgin_ptr,
           short      *restrict imgout_ptr,
           short                   width,
           short                   pitch,
    const  short    *restrict   mask_ptr,
           short                   shift

)
{

    /* -------------------------------------------------------------------- */
    /* in0 to in4 contains pointers to lines 1 to 5                         */
    /* -------------------------------------------------------------------- */

    const short *restrict   in0;
    const short *restrict   in1;
    const short *restrict   in2;
    const short *restrict   in3;
    const short *restrict   in4;

    int             i,              count;
    __int40_t       sum0,           sum1;

    double          dm1;

    long long       dp1,            dp2;
    long long       row1_3210,      row2_3210,      row3_3210;
    long long       row4_3210,      row5_3210,      row1_5432;
    long long       row2_5432,      row3_5432,      row4_5432;
    long long       row5_5432;
    long long       mask1_3210,     mask2_3210,     mask3_3210;
    long long       mask4_3210,     mask5_3210;

    unsigned int    row1_54,        row2_54,        row3_54;
    unsigned int    row4_54,        row5_54;
    unsigned int    mask1_44,       mask2_44,       mask3_44;
    unsigned int    mask4_44,       mask5_44;

    /* -------------------------------------------------------------------- */
    /*                           Load mask values                           */
    /* -------------------------------------------------------------------- */

    mask1_3210 = _mem8_const( ( void* ) &mask_ptr[0]);
    mask1_44   = _pack2((int)mask_ptr[4], (int)mask_ptr[4]);

    mask2_3210 = _mem8_const( ( void* ) &mask_ptr[5]);
    mask2_44   = _pack2((int)mask_ptr[9], (int)mask_ptr[9]);

    mask3_3210 = _mem8_const( ( void* ) &mask_ptr[10]);
    mask3_44   = _pack2((int)mask_ptr[14], (int)mask_ptr[14]);

    mask4_3210 = _mem8_const( ( void* ) &mask_ptr[15]);
    mask4_44   = _pack2((int)mask_ptr[19], (int)mask_ptr[19]);

    mask5_3210 = _mem8_const( ( void* ) &mask_ptr[20]);
    mask5_44   = _pack2((int)mask_ptr[24], (int)mask_ptr[24]);

    /* -------------------------------------------------------------------- */
    /* initialise pointers corresponding to each row                        */
    /* -------------------------------------------------------------------- */

    in0 = imgin_ptr ;
    in1 = in0 + pitch;
    in2 = in1 + pitch;
    in3 = in2 + pitch;
    in4 = in3 + pitch;

    /* -------------------------------------------------------------------- */
    /* load the number of times loop to be executed into count              */
    /* -------------------------------------------------------------------- */

    count =  width  >> 1;

    /* -------------------------------------------------------------------- */
    /* two output pixels are calculated in each iteration                   */
    /* -------------------------------------------------------------------- */

    _nassert((int)imgout_ptr % 4 == 0);  /* Output Dword aligned            */
    _nassert((int)(width)    % 2 == 0);  /* output samples multiple of 2    */

    for (i = 0; i < count ; i++)
    {
        sum0 = 0;
        sum1 = 0;

        /* ---------------------------------------------------------------- */
        /*load pixel values for row 1,calculate partial sum of products     */
        /* ---------------------------------------------------------------- */

        row1_3210   =   _mem8_const(in0);
        row1_54     =   _mem4_const(in0 + 4);
        row1_5432   =   _itoll(row1_54, _hill(row1_3210));

        dp1         =   _ddotpl2(row1_3210, _loll(mask1_3210) );
        dp2         =   _ddotpl2(row1_5432, _hill(mask1_3210));
        dm1         =   _mpy2( row1_54, mask1_44);

        sum0       +=   ((signed int) _loll(dp1));
        sum0       +=   ((signed int) _loll(dp2));
        sum0       +=   ((signed int) _lo(dm1));

        sum1       +=   ((signed int) _hill(dp1));
        sum1       +=   ((signed int) _hill(dp2));
        sum1       +=   ((signed int) _hi(dm1));

        in0         =   in0 + 2;

        /* ---------------------------------------------------------------- */
        /*load pixel values for row 2,calculate partial sum of products     */
        /* ---------------------------------------------------------------- */

        row2_3210   =   _mem8_const(in1);
        row2_54     =   _mem4_const(in1 + 4);
        row2_5432   =   _itoll( row2_54, _hill(row2_3210) );

        dp1         =   _ddotpl2( row2_3210, _loll(mask2_3210) );
        dp2         =   _ddotpl2( row2_5432 , _hill(mask2_3210));
        dm1         =   _mpy2( row2_54, mask2_44 );

        sum0       +=   ((signed int) _loll(dp1));
        sum0       +=   ((signed int) _loll(dp2));
        sum0       +=   ((signed int) _lo(dm1));

        sum1       +=   ((signed int) _hill(dp1));
        sum1       +=   ((signed int) _hill(dp2));
        sum1       +=   ((signed int) _hi(dm1));

        in1         =   in1 + 2;

        /* ---------------------------------------------------------------- */
        /*load pixel values for row 3,calculate partial sum of products     */
        /* ---------------------------------------------------------------- */

        row3_3210   =   _mem8_const(in2);
        row3_54     =   _mem4_const(in2+4);
        row3_5432   =   _itoll( row3_54, _hill(row3_3210) );

        dp1         =   _ddotpl2( row3_3210, _loll(mask3_3210) );
        dp2         =   _ddotpl2( row3_5432 , _hill(mask3_3210));
        dm1         =   _mpy2( row3_54, mask3_44 );

        sum0       +=   ((signed int) _loll(dp1));
        sum0       +=   ((signed int) _loll(dp2));
        sum0       +=   ((signed int) _lo(dm1));

        sum1       +=   ((signed int) _hill(dp1));
        sum1       +=   ((signed int) _hill(dp2));
        sum1       +=   ((signed int) _hi(dm1));

        in2         =   in2 + 2;

        /* ---------------------------------------------------------------- */
        /*load pixel values for row 4,calculate partial sum of products     */
        /* ---------------------------------------------------------------- */

        row4_3210   =   _mem8_const(in3);
        row4_54     =   _mem4_const(in3+4);
        row4_5432   =   _itoll( row4_54, _hill(row4_3210) );

        dp1         =   _ddotpl2( row4_3210, _loll (mask4_3210) );
        dp2         =   _ddotpl2( row4_5432 , _hill(mask4_3210));
        dm1         =   _mpy2( row4_54, mask4_44 );

        sum0       +=   ((signed int) _loll(dp1));
        sum0       +=   ((signed int) _loll(dp2));
        sum0       +=   ((signed int) _lo(dm1));

        sum1       +=   ((signed int) _hill(dp1));
        sum1       +=   ((signed int) _hill(dp2));
        sum1       +=   ((signed int) _hi(dm1));

        in3         =   in3 + 2;

        /* ---------------------------------------------------------------- */
        /*load pixel values for row 5,calculate partial sum of products     */
        /* ---------------------------------------------------------------- */

        row5_3210   =   _mem8_const(in4);
        row5_54     =   _mem4_const(in4 + 4);
        row5_5432   =   _itoll( row5_54, _hill(row5_3210) );

        dp1         =   _ddotpl2( row5_3210, _loll(mask5_3210) );
        dp2         =   _ddotpl2( row5_5432 , _hill(mask5_3210));

        dm1         =   _mpy2( row5_54, mask5_44 );

        sum0       +=   ((signed int) _loll(dp1));
        sum0       +=   ((signed int) _loll(dp2));
        sum0       +=   ((signed int) _lo(dm1));

        sum1       +=   ((signed int) _hill(dp1));
        sum1       +=   ((signed int) _hill(dp2));
        sum1       +=   ((signed int) _hi(dm1));

        in4         =   in4 + 2;

        /* ---------------------------------------------------------------- */
        /* shift the sum approptiately to avoid overflow                    */
        /* ---------------------------------------------------------------- */

        sum0        =    (sum0 >> shift);
        sum1        =    (sum1 >> shift);
        _amem4( ( void * )&imgout_ptr[i] ) = _spack2(sum1, sum0);



    }
}

/* ======================================================================== */
/*  End of file:  IMG_corr_5x5_i16s_c16s.c                                  */
/* ======================================================================== */
