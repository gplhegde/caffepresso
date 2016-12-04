#ifndef _CONV_LAYER_H
#define _CONV_LAYER_H
#include "struct_defs.h"
#include "user_config.h"

//=============================================================
//=============================================================
// Convolution layer parameter info
typedef struct {
	// Input map height
	int map_h;
	// Input map width
	int map_w;
	// Kernel size of the conv layer
	int ker_size;
	// No of input maps to this layer
	int no_inputs;
	// No of output maps produced by this layer
	int no_outputs;
	// horizontal and vertical pad
	int pad;
	// horizontal and vertical pad
	int stride;
	// No of fraction bits in kernel coefficient representation
	int no_ker_frac_bits;
	// No of fraction bits in map pixel representation
	int no_map_frac_bits;
} CONV_INFO_T;
//=============================================================
//=============================================================
// Convolution layer context
typedef struct {
	// layer arithmetic mode. floating point or fixed point
	LYR_ARITH_MODE_E lyr_arith_mode;	// This must be the first member
	// Floating point output map buffer for this layer.
	FLT_MAP *p_flt_output;
	// Fixed point output map buffer for this layer.
	FIX_MAP *p_fix_output;
	// Floating point kernels
	FLT_KER *p_flt_ker;
	// Fixed point kernel buffer
	FIX_KER *p_fix_ker;
	// Floating point bias buffer
	FLT_KER *p_flt_bias;
	// Fixed point bias buffer
	FIX_KER *p_fix_bias;
	// starting output map assigned to this core. this number is zero based
	uint16_t start_map[NO_CORES];
	// number of output maps assigned to this core.
	uint16_t no_maps[NO_CORES];
	// Layer parameters.
	CONV_INFO_T conv_info;
} CONV_LYR_CTX_T;
//=============================================================
//=============================================================


STATUS_E dsp_conv_layer(CONV_LYR_CTX_T *p_conv_ctx, FLT_MAP *p_flt_in_maps, FIX_MAP *p_fix_in_map);
#endif // _CONV_LAYER_H
