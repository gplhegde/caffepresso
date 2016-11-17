#ifndef _CONV_LAYER_H
#define _CONV_LAYER_H
#include "struct_defs.h"
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
	// starting output map assigned to this core. this number is zero based
	int start_map;
	// ending output map assigned to this core. this number is zero based
	int end_map;
} CONV_INFO_T;
//=============================================================
//=============================================================
// Convolution layer context
typedef struct {
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
	// layer arithmetic mode. floating point or fixed point
	LYR_ARITH_MODE_E lyr_arith_mode;
	// Layer parameters.
	CONV_INFO_T conv_info;
} CONV_LYR_CTX_T;
//=============================================================
//=============================================================


STATUS_E dsp_conv_layer(FIX_MAP *p_input,	// pointer to input maps stored in flattened [maps][row][col] format.
	FIX_KER *p_weight,	// pointer to kernels stored in flattened [no_outputs][no_inputs][ker_size][ker_size] format
	FIX_KER *p_bias,	// pointer to bias units. there are 'no_outputs' bias units
	int in_height,		// input feature map height
	int in_width,		// input feature map width
	int no_inputs,		// number of input feature maps
	int no_outputs,		// number of output feature maps
	int ker_size,		// kernel size. We support only square sized kernels
	int stride,			// convolution window stride in both horizontal and vertical direction.
	int shift,			// Shifts used for 16b fixed point conversion. Perform shift before adding bias.
	FIX_MAP *p_output	// pointer to output feature maps. Stored in [map][row][col] flattened manner.
	);

#endif // _CONV_LAYER_H
