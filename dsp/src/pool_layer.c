#include "pool_layer.h"
#include "struct_defs.h"
#include <float.h>

STATUS_E dsp_fix_pool_layer(FIX_MAP *p_input,	// pointer to input maps stored in flattened [maps][row][col] format.
	int in_height,			// input feature map height
	int in_width,			// input feature map width
	int no_inputs,			// number of input feature maps
	int no_outputs,			// number of output feature maps
	int win_size,			// kernel size. We support only square sized kernels
	int stride,				// convolution window stride in both horizontal and vertical direction.
	int pad,				// padding on all 4 sides of feature map
	POOL_TYPE_E pool_type,	// pooling type, max pooling or average pooling
	FIX_MAP *p_output		// pointer to output feature maps. Stored in [map][row][col] flattened manner.
	) {

	STATUS_E status = FAILED;
	int map, o_w, o_h, row, col, i, j;
	FIX_MAP max;
	int sum;


	o_h = (in_height + 2 * pad - win_size + 1 + stride - 1)/ stride;
	o_w = (in_width + 2 * pad - win_size + 1 + stride - 1)/ stride;

	switch(pool_type) {
		case MAX_POOL:
			for (map = 0; map < no_inputs; map++) {
				for(row = 0; row < in_height - win_size + 1; row += stride) {
					for(col = 0; col < in_width - win_size + 1; col += stride) {
						max = -32768; // TODO: define -ve min of FP_MAP_PIXEL type and use here.
						for ( i = 0; i < win_size; i++) {
							for ( j = 0; j < win_size; j++) {
								max = MAX(max, p_input[map * in_width * in_height + (row + i) * in_width + col + j]);
							}
						}
						p_output[map * o_w * o_h + o_w * (row / stride) + (col / stride)] = max;
					}
				}
			}
			break;
		case AVG_POOL:
			for (map = 0; map < no_inputs; map++) {
				for(row = 0; row < in_height; row += stride) {
					for(col = 0; col < in_width; col += stride) {
						sum = 0;
						for ( i = 0; i < win_size; i++) {
							for ( j = 0; j < win_size; j++) {
								sum += p_input[map * in_width * in_height + (row + i) * in_width + col + j];
							}
						}
						p_output[map * o_w * o_h + o_w * (row / stride) + (col / stride)] =
							(FIX_MAP)(sum / (win_size * win_size));
					}
				}
			}			
			break;
		default:
			break;
	}
	return status;
}

STATUS_E dsp_flt_pool_layer(FLT_MAP *p_input,	// pointer to input maps stored in flattened [maps][row][col] format.
	int in_height,			// input feature map height
	int in_width,			// input feature map width
	int no_inputs,			// number of input feature maps
	int no_outputs,			// number of output feature maps
	int win_size,			// kernel size. We support only square sized kernels
	int stride,				// convolution window stride in both horizontal and vertical direction.
	int pad,				// padding on all 4 sides of feature map
	POOL_TYPE_E pool_type,	// pooling type, max pooling or average pooling
	FLT_MAP *p_output		// pointer to output feature maps. Stored in [map][row][col] flattened manner.
	) {
		
	int map, o_w, o_h, row, col, i, j;
	FLT_MAP max;
	int sum;


	o_h = (in_height + 2 * pad - win_size + 1 + stride - 1)/ stride;
	o_w = (in_width + 2 * pad - win_size + 1 + stride - 1)/ stride;
	STATUS_E status = FAILED;
	
	switch(pool_type) {
		case MAX_POOL:
			for (map = 0; map < no_inputs; map++) {
				for(row = 0; row < in_height - win_size + 1; row += stride) {
					for(col = 0; col < in_width - win_size + 1; col += stride) {
						max = -FLT_MAX; // TODO: define -ve min of FP_MAP_PIXEL type and use here.
						for ( i = 0; i < win_size; i++) {
							for ( j = 0; j < win_size; j++) {
								max = MAX(max, p_input[map * in_width * in_height + (row + i) * in_width + col + j]);
							}
						}
						p_output[map * o_w * o_h + o_w * (row / stride) + (col / stride)] = max;
					}
				}
			}
			break;
		case AVG_POOL:
			for (map = 0; map < no_inputs; map++) {
				for(row = 0; row < in_height; row += stride) {
					for(col = 0; col < in_width; col += stride) {
						sum = 0;
						for ( i = 0; i < win_size; i++) {
							for ( j = 0; j < win_size; j++) {
								sum += p_input[map * in_width * in_height + (row + i) * in_width + col + j];
							}
						}
						p_output[map * o_w * o_h + o_w * (row / stride) + (col / stride)] =
							(FLT_MAP)(sum / (win_size * win_size));
					}
				}
			}			
			break;
		default:
			break;
	}
	return status;
}

STATUS_E dsp_pool_layer(POOL_LYR_CTX_T *p_pool_ctx, FLT_MAP *p_flt_in_maps, FIX_MAP *p_fix_in_maps) {
	STATUS_E status = FAILED;

	if(p_pool_ctx->lyr_arith_mode == FIXED_POINT) {
		status = dsp_fix_pool_layer(p_fix_in_maps,
			p_pool_ctx->pool_info.map_w,
			p_pool_ctx->pool_info.map_h,
			p_pool_ctx->pool_info.no_inputs,
			p_pool_ctx->pool_info.no_outputs,
			p_pool_ctx->pool_info.win_size,
			p_pool_ctx->pool_info.stride,
			p_pool_ctx->pool_info.pad,
			p_pool_ctx->pool_info.pool_type,
			p_pool_ctx->p_fix_output
			);
	} else {
		status = dsp_flt_pool_layer(p_flt_in_maps,
			p_pool_ctx->pool_info.map_w,
			p_pool_ctx->pool_info.map_h,
			p_pool_ctx->pool_info.no_inputs,
			p_pool_ctx->pool_info.no_outputs,
			p_pool_ctx->pool_info.win_size,
			p_pool_ctx->pool_info.stride,
			p_pool_ctx->pool_info.pad,
			p_pool_ctx->pool_info.pool_type,
			p_pool_ctx->p_flt_output
			);
	}

	return status;
}
