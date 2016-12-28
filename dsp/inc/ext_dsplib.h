/*
 * ext_dsplib.h
 *
 *  Created on: 23 Dec 2016
 *      Author: hgashok
 */

#ifndef INC_EXT_DSPLIB_H_
#define INC_EXT_DSPLIB_H_

void DSP_vs_add16_unroll_4 (
    short * restrict x,   /* Input array of length nx  */
    short y,   			  /* Scalar value to be added */
    short * restrict r,   /* Output array of length nx. even though the output array can be same as input in this case(because it is pointwise op) using restrict to fool the compiler*/
    int              nx   /* Number of elements.       */
);

void DSPF_vs_add_unroll_4 (
    float * restrict x,   /* Input array of length nx  */
    float y,   			  /* Scalar to be added */
    float * restrict r,   /* Output array of length nx */
    int              nx   /* Number of elements.       */
);

void DSP_vs_add_unroll_8 (
    short * restrict x,   /* Input array of length nx  */
    short y,   			  /* Scalar to be added */
    int              nx   /* Number of elements.       */
);

void DSP_vector_scale_translate(
		short * restrict x,
		short * restrict y,
		short s,
		short t,
		int nx,
		short shift
	);

void DSP_fix_max_pool(
		short * restrict p_mat, // must be 8 byte aligned
		int n_rows,
		int n_cols,		//must be multiple of 8
		short *restrict p_out
		);

void IMG_corr_3x3_i16s_c16s_short
(
    const  short    *restrict  imgin_ptr,
           short      *restrict imgout_ptr,
           short                   width,
           short                   pitch,
    const  short    *restrict   mask_ptr,
           short                   shift
);

void IMG_corr_5x5_i16s_c16s_short
(
    const  short    *restrict  imgin_ptr,
           short      *restrict imgout_ptr,
           short                   width,
           short                   pitch,
    const  short    *restrict   mask_ptr,
           short                   shift

);

void IMG_corr_11x11_i16s_c16s_short
(
    const  short    *restrict  imgin_ptr,
           short      *restrict imgout_ptr,
           short                   width,
           short                   pitch,
    const  short    *restrict   mask_ptr,
           short                   shift
);
#endif /* INC_EXT_DSPLIB_H_ */
