/*
 * batch_norm_layer.c
 *
 *  Created on: 15 Dec 2016
 *      Author: Gopalakrishna Hegde
 */
#include "batch_norm_layer.h"
#include "ext_dsplib.h"
#include <assert.h>
extern unsigned int core_id;

STATUS_E dsp_fix_batch_norm_layer(FIX_MAP *p_input,
	int map_h, int map_w, int no_maps, FIX_KER *p_scale,
	FIX_KER *p_offset, int shift, FIX_MAP *p_output) {

	int map;
	STATUS_E status;
	for(map = 0; map < no_maps; map++) {
// TODO: double buffering with EDMA
		DSP_vector_scale_translate(p_input + map * map_h * map_w,
			p_output + map * map_h * map_w, p_scale[map], p_offset[map], map_h * map_w, shift);

	}
	status = SUCCESS;
	return status;
}

STATUS_E dsp_flt_batch_norm_layer(FLT_MAP *p_input,
	int map_h, int map_w, int no_maps, FLT_KER *p_scale,
	FLT_KER *p_offset, FLT_MAP *p_output) {

	int map, e;
	STATUS_E status;
	for(map = 0; map < no_maps; map++) {
		for(e = 0; e < map_h * map_w; e++) {
			p_output[map * map_h * map_w + e] = p_input[map * map_h * map_w + e] * p_scale[map] + p_offset[map];
		}
	}
	status = SUCCESS;
	return status;
}

STATUS_E dsp_batch_norm_layer(BNORM_LYR_CTX_T *p_bnorm_ctx, FLT_MAP *p_flt_in_maps, FIX_MAP *p_fix_in_maps) {
	STATUS_E status;
	unsigned int buff_offset;
	status = FAILED;
	buff_offset = p_bnorm_ctx->bnorm_info.map_h * p_bnorm_ctx->bnorm_info.map_w * p_bnorm_ctx->start_map[core_id];

	if(p_bnorm_ctx->lyr_arith_mode == FIXED_POINT) {
		dsp_fix_batch_norm_layer(p_fix_in_maps + buff_offset,
				p_bnorm_ctx->bnorm_info.map_h,
				p_bnorm_ctx->bnorm_info.map_w,
				p_bnorm_ctx->no_maps[core_id],
				p_bnorm_ctx->p_fix_scale + p_bnorm_ctx->start_map[core_id],
				p_bnorm_ctx->p_fix_offset + p_bnorm_ctx->start_map[core_id],
				p_bnorm_ctx->bnorm_info.no_ker_frac_bits,
				p_bnorm_ctx->p_fix_output + buff_offset
			);
	} else {
		dsp_flt_batch_norm_layer(p_flt_in_maps + buff_offset,
				p_bnorm_ctx->bnorm_info.map_h,
				p_bnorm_ctx->bnorm_info.map_w,
				p_bnorm_ctx->no_maps[core_id],
				p_bnorm_ctx->p_flt_scale + p_bnorm_ctx->start_map[core_id],
				p_bnorm_ctx->p_flt_offset + p_bnorm_ctx->start_map[core_id],
				p_bnorm_ctx->p_flt_output + buff_offset
			);
	}
	status = SUCCESS;
	return status;
}
