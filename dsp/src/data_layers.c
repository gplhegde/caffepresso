/*
 * data_layers.c
 *
 *  Created on: 15 Dec 2016
 *      Author: Gopalakrishna Hegde
 */
#include "struct_defs.h"
#include <string.h>

// Taken from Caffe implementation
// https://github.com/BVLC/caffe/blob/master/src/caffe/util/im2col.cpp
inline uint32_t is_a_ge_zero_and_a_lt_b(int a, int b) {
  return (unsigned int)a < (unsigned int)b;
}

// Convert feature maps into a single 2D matrix with unrolled strided windows
// Each column of the matrix will contain 1 3D window in its unrolled form.
void fix_img2col(FIX_MAP *p_img, FIX_MAP *p_mat,
	const int n_ch, const int in_h, const int in_w,
	const int K, const int stride, const int pad, const int zero_rows, const int zero_cols) {

	int out_h, out_w, out_row, out_col, in_row, in_col, ch, kr, kc, mat_w;
	out_h = (in_h + 2*pad - K) / stride + 1;
	out_w = (in_w + 2*pad - K) / stride + 1;

	mat_w = out_h * out_w + zero_cols;

	for(ch = 0; ch < n_ch; ch++) {
		for(kr = 0; kr < K; kr++) {
			for(kc = 0; kc < K; kc++) {
				in_row = -pad + kr;
				for(out_row = 0; out_row < out_h; out_row++) {
					if(!is_a_ge_zero_and_a_lt_b(in_row, in_h)) {
						for(out_col = 0; out_col < out_w; out_col++) {
							*(p_mat++) = 0;
						}
					} else {
						in_col = -pad + kc;
						for(out_col = 0; out_col < out_w; out_col++) {
							if(is_a_ge_zero_and_a_lt_b(in_col, in_w)) {
								*(p_mat++) = p_img[in_row * in_w + in_col];
							} else {
								*(p_mat++) = 0;
							}
							in_col += stride;
						}
					}
					in_row += stride;
				}
				// pad extra columns to the matrix if necessary to make it multiple of something
				for(out_col = 0; out_col  < zero_cols; out_col++) {
					*(p_mat++) = 0;
				}
			}
		}
		p_img += (in_w * in_h);
	}
	// pad extra rows to the matrix if necessary
	for(out_row = 0; out_row < zero_rows; out_row++) {
		for(out_col = 0; out_col < mat_w; out_col++) {
			*(p_mat++) = 0;
		}
	}
}

void flt_img2col(FLT_MAP *p_img, FLT_MAP *p_mat,
	const int n_ch, const int in_h, const int in_w,
	const int K, const int stride, const int pad, const int zero_rows, const int zero_cols) {

	int out_h, out_w, out_row, out_col, in_row, in_col, ch, kr, kc, mat_w;
	out_h = (in_h + 2*pad - K) / stride + 1;
	out_w = (in_w + 2*pad - K) / stride + 1;

	mat_w = out_h * out_w + zero_cols;

	for(ch = 0; ch < n_ch; ch++) {
		for(kr = 0; kr < K; kr++) {
			for(kc = 0; kc < K; kc++) {
				in_row = -pad + kr;
				for(out_row = 0; out_row < out_h; out_row++) {
					if(!is_a_ge_zero_and_a_lt_b(in_row, in_h)) {
						for(out_col = 0; out_col < out_w; out_col++) {
							*(p_mat++) = 0;
						}
					} else {
						in_col = -pad + kc;
						for(out_col = 0; out_col < out_w; out_col++) {
							if(is_a_ge_zero_and_a_lt_b(in_col, in_w)) {
								*(p_mat++) = p_img[in_row * in_w + in_col];
							} else {
								*(p_mat++) = 0;
							}
							in_col += stride;
						}
					}
					in_row += stride;
				}
				// pad extra columns to the matrix if necessary to make it multiple of something
				for(out_col = 0; out_col  < zero_cols; out_col++) {
					*(p_mat++) = 0;
				}
			}
		}
		p_img += (in_w * in_h);
	}
	// pad extra rows to the matrix if necessary
	for(out_row = 0; out_row < zero_rows; out_row++) {
		for(out_col = 0; out_col < mat_w; out_col++) {
			*(p_mat++) = 0;
		}
	}
}
void fix_zeropad(FIX_MAP *p_src, FIX_MAP *p_dst,
	int n_ch, int src_h, int src_w, int pad_h, int pad_w) {

	int dst_h, dst_w, ch, r, c;
	dst_h = src_h + 2*pad_h;
	dst_w = src_w + 2*pad_w;

	for(ch = 0; ch < n_ch; ch++) {
		for(r = 0; r < src_h + 2*pad_h; r++) {
			for(c = 0; c < src_w + 2*pad_w; c++) {
				if(r < pad_h || (r > src_h+pad_h-1) || c < pad_w || (c > src_w+pad_w-1)) {
					p_dst[ch*dst_h*dst_w + r*dst_w + c] = 0;
				}else {
					// TODO: use memcpy instead to do bulk copy of one full row
					p_dst[ch*dst_h*dst_w + r*dst_w + c] = p_src[ch*src_h*src_w + (r-pad_h)*src_w + c-pad_w];
				}
			}
		}
	}
}

void flt_zeropad(FLT_MAP *p_src, FLT_MAP *p_dst,
	int n_ch, int src_h, int src_w, int pad_h, int pad_w) {

	int dst_h, dst_w, ch, r, c;
	dst_h = src_h + 2*pad_h;
	dst_w = src_w + 2*pad_w;

	for(ch = 0; ch < n_ch; ch++) {
		for(r = 0; r < src_h + 2*pad_h; r++) {
			for(c = 0; c < src_w + 2*pad_w; c++) {
				if(r < pad_h || (r > src_h+pad_h-1) || c < pad_w || (c > src_w+pad_w-1)) {
					p_dst[ch*dst_h*dst_w + r*dst_w + c] = (FLT_MAP)0;
				}else {
					// TODO: use memcpy instead to do bulk copy of one full row
					p_dst[ch*dst_h*dst_w + r*dst_w + c] = p_src[ch*src_h*src_w + (r-pad_h)*src_w + c-pad_w];
				}
			}
		}
	}
}

void fix_post_zero_pad(FIX_MAP *p_src, int H, int W, int row_pad, int col_pad, FIX_MAP *p_dst) {
	int r, c;
	for(r = 0; r < H; r++) {
		for(c = 0; c < W; c++) {
			*(p_dst++) = p_src[r * W + c];
		}
		for(c = 0; c < col_pad; c++) {
			*(p_dst++) = 0;
		}
	}
	memset(p_dst, 0, row_pad * (W + col_pad) * sizeof(FIX_MAP));
}

void flt_post_zero_pad(FLT_MAP *p_src, int H, int W, int row_pad, int col_pad, FLT_MAP *p_dst) {
	int r, c;
	for(r = 0; r < H; r++) {
		for(c = 0; c < W; c++) {
			*(p_dst++) = p_src[r * W + c];
		}
		for(c = 0; c < col_pad; c++) {
			*(p_dst++) = 0;
		}
	}
	memset(p_dst, 0, row_pad * (W + col_pad) * sizeof(FLT_MAP));
}
