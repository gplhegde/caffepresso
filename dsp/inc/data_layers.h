/*
 * data_layers.h
 *
 *  Created on: 15 Dec 2016
 *      Author: hgashok
 */

#ifndef INC_DATA_LAYERS_H_
#define INC_DATA_LAYERS_H_
#include "struct_defs.h"

void fix_img2col(FIX_MAP *p_img, FIX_MAP *p_mat,
	const int n_ch, const int in_h, const int in_w,
	const int K, const int stride, const int pad, const int zero_rows, const int zero_cols);

void fix_zeropad(FIX_MAP *p_src, FIX_MAP *p_dst,
	int n_ch, int src_h, int src_w, int pad_h, int pad_w);

void fix_post_zero_pad(FIX_MAP *p_src, int H, int W, int row_pad, int col_pad, FIX_MAP *p_dst);

void flt_img2col(FLT_MAP *p_img, FLT_MAP *p_mat,
	const int n_ch, const int in_h, const int in_w,
	const int K, const int stride, const int pad, const int zero_rows, const int zero_cols);

void flt_zeropad(FLT_MAP *p_src, FLT_MAP *p_dst,
	int n_ch, int src_h, int src_w, int pad_h, int pad_w);

void flt_post_zero_pad(FIFLT_MAPX_MAP *p_src, int H, int W, int row_pad, int col_pad, FLT_MAP *p_dst);
#endif /* INC_DATA_LAYERS_H_ */
