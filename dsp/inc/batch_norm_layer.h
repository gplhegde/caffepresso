/*
 * batch_norm_layer.h
 *
 *  Created on: 15 Dec 2016
 *      Author: Gopalakrishna Hegde
 */

#ifndef INC_BATCH_NORM_LAYER_H_
#define INC_BATCH_NORM_LAYER_H_
#include "struct_defs.h"
#include "user_config.h"
//=================================================
//=================================================
// Pooling layer parameter structure.
typedef struct {
	// input map height
	int map_h;
	// input map width
	int map_w;
	// No of input maps to this layer
	int no_inputs;
	// No of output maps produced by this layer
	int no_outputs;
	// No of fraction bits in scale and offset coefficient representation
	int no_ker_frac_bits;
	// No of fraction bits in map pixel representation
	int no_map_frac_bits;
} BNORM_INFO_T;

//=================================================
//=================================================
// Pooling layer context
typedef struct {
	// Arithmetic mode to be used to compute the output. float/fixed point
	LYR_ARITH_MODE_E lyr_arith_mode; // This must be the first member
	// Floating point output map buffer for this layer.
	FLT_MAP *p_flt_output;
	// Fixed point output map buffer for this layer.
	FIX_MAP *p_fix_output;
	// Fixed point scaling factor. Scale = inverse_std * gamma. This is pre-computed before the inference phase
	FIX_KER *p_fix_scale;
	// Fixed point offset. , Offset = (beta - mu * gamma * inverse_std), This is pre-computed before the inference phase.
	FIX_KER *p_fix_offset;
	// Float point scaling factor
	FLT_KER *p_flt_scale;
	// Float point offset
	FLT_KER *p_flt_offset;
	// starting output map assigned to this core. this number is zero based
	uint16_t start_map[NO_CORES];
	// number of output maps assigned to this core.
	uint16_t no_maps[NO_CORES];
	// Pooling layer parameters
	BNORM_INFO_T bnorm_info;
} BNORM_LYR_CTX_T;
//=================================================
//=================================================


STATUS_E dsp_batch_norm_layer(BNORM_LYR_CTX_T *p_bnorm_ctx, FLT_MAP *p_flt_in_maps, FIX_MAP *p_fix_in_maps);

#endif /* INC_BATCH_NORM_LAYER_H_ */
