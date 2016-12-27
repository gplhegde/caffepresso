#ifndef _CONV_LAYER_H
#define _CONV_LAYER_H
#include "struct_defs.h"
#include "user_config.h"
#include <ti/csl/cslr_device.h>

#define MAX_SUPPORTED_KER_SIZE 		(11)

#define MAX_INPUT_MAP_WIDTH			(256)

#define MAX_SUPPORTED_INPUT_MAPS	(512)
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
inline void strided_move(FIX_MAP *p_input, int len, int stride) {
	int col, i;
	for(i = 0, col = 0; i < len; i++, col += stride) {
		p_input[i] = p_input[col];
	}
}

inline Bool is_dram_addr(Uint32 addr) {
	return addr >= CSL_DDR3_0_DATA;
}

inline Bool is_on_chip_addr(Uint32 addr) {
	return addr < CSL_DDR3_0_DATA;
}

STATUS_E dsp_fix_conv_11x11(FIX_MAP *p_input,	// pointer to input maps stored in flattened [maps][row][col] format.
	FIX_KER *p_weight,	// pointer to kernels stored in flattened [no_outputs][no_inputs][ker_size][ker_size] format
	FIX_KER *p_bias,	// pointer to bias units. there are 'no_outputs' bias units
	int in_height,		// input feature map height
	int in_width,		// input feature map width
	int no_inputs,		// number of input feature maps
	int no_outputs,		// number of output feature maps
	int start_map,		// map offset to start fot this core.
	int no_maps, 		// no of feature maps assigned to this core.
	int pad,			// padding in all 4 sides. Only equal padding in all dimensions is supported.
	int stride,			// convolution window stride in both horizontal and vertical direction.
	int shift,			// Shifts used for 16b fixed point conversion. Perform shift before adding bias.
	FIX_MAP *p_output	// pointer to output feature maps. Stored in [map][row][col] flattened manner.
	);

// Use this when pad == 0 AND weights are stored in on-chip memory
STATUS_E dsp_fix_conv_11x11_constrained(FIX_MAP *p_input,	// pointer to input maps stored in flattened [maps][row][col] format.
	FIX_KER *p_weight,	// pointer to kernels stored in flattened [no_outputs][no_inputs][ker_size][ker_size] format
	FIX_KER *p_bias,	// pointer to bias units. there are 'no_outputs' bias units
	int in_height,		// input feature map height
	int in_width,		// input feature map width
	int no_inputs,		// number of input feature maps
	int no_outputs,		// number of output feature maps
	int start_map,		// map offset to start fot this core.
	int no_maps, 		// no of feature maps assigned to this core.
	int stride,			// convolution window stride in both horizontal and vertical direction.
	int shift,			// Shifts used for 16b fixed point conversion. Perform shift before adding bias.
	FIX_MAP *p_output	// pointer to output feature maps. Stored in [map][row][col] flattened manner.
	);

// Use this API when the weights are stored in DDR OR pad != 0
STATUS_E dsp_fix_conv_7x7(FIX_MAP *p_input,	// pointer to input maps stored in flattened [maps][row][col] format.
	FIX_KER *p_weight,	// pointer to kernels stored in flattened [no_outputs][no_inputs][ker_size][ker_size] format
	FIX_KER *p_bias,	// pointer to bias units. there are 'no_outputs' bias units
	int in_height,		// input feature map height
	int in_width,		// input feature map width
	int no_inputs,		// number of input feature maps
	int no_outputs,		// number of output feature maps
	int start_map,		// map offset to start fot this core.
	int no_maps, 		// no of feature maps assigned to this core.
	int pad,			// padding in all 4 sides. Only equal padding in all dimensions is supported.
	int stride,			// convolution window stride in both horizontal and vertical direction.
	int shift,			// Shifts used for 16b fixed point conversion. Perform shift before adding bias.
	FIX_MAP *p_output	// pointer to output feature maps. Stored in [map][row][col] flattened manner.
	);

STATUS_E dsp_fix_conv_5x5(FIX_MAP *p_input,	// pointer to input maps stored in flattened [maps][row][col] format.
	FIX_KER *p_weight,	// pointer to kernels stored in flattened [no_outputs][no_inputs][ker_size][ker_size] format
	FIX_KER *p_bias,	// pointer to bias units. there are 'no_outputs' bias units
	int in_height,		// input feature map height
	int in_width,		// input feature map width
	int no_inputs,		// number of input feature maps
	int no_outputs,		// number of output feature maps
	int start_map,		// map offset to start fot this core.
	int no_maps, 		// no of feature maps assigned to this core.
	int pad,			// padding in all 4 sides. Only equal padding in all dimensions is supported.
	int stride,			// convolution window stride in both horizontal and vertical direction.
	int shift,			// Shifts used for 16b fixed point conversion. Perform shift before adding bias.
	FIX_MAP *p_output	// pointer to output feature maps. Stored in [map][row][col] flattened manner.
	);

// Use this when pad == 0 AND in_width % 4 == 0 AND weights are stored in on-chip memory
STATUS_E dsp_fix_conv_7x7_constrained(FIX_MAP *p_input,	// pointer to input maps stored in flattened [maps][row][col] format.
	FIX_KER *p_weight,	// pointer to kernels stored in flattened [no_outputs][no_inputs][ker_size][ker_size] format
	FIX_KER *p_bias,	// pointer to bias units. there are 'no_outputs' bias units
	int in_height,		// input feature map height
	int in_width,		// input feature map width
	int no_inputs,		// number of input feature maps
	int no_outputs,		// number of output feature maps
	int start_map,		// map offset to start fot this core.
	int no_maps, 		// no of feature maps assigned to this core.
	int stride,			// convolution window stride in both horizontal and vertical direction.
	int shift,			// Shifts used for 16b fixed point conversion. Perform shift before adding bias.
	FIX_MAP *p_output	// pointer to output feature maps. Stored in [map][row][col] flattened manner.
	);

// Use this when pad == 0 AND in_width % 2 == 0 AND weights are stored in on-chip memory
STATUS_E dsp_fix_conv_5x5_constrained(FIX_MAP *p_input,	// pointer to input maps stored in flattened [maps][row][col] format.
	FIX_KER *p_weight,	// pointer to kernels stored in flattened [no_outputs][no_inputs][ker_size][ker_size] format
	FIX_KER *p_bias,	// pointer to bias units. there are 'no_outputs' bias units
	int in_height,		// input feature map height
	int in_width,		// input feature map width
	int no_inputs,		// number of input feature maps
	int no_outputs,		// number of output feature maps
	int start_map,		// map offset to start fot this core.
	int no_maps, 		// no of feature maps assigned to this core.
	int stride,			// convolution window stride in both horizontal and vertical direction.
	int shift,			// Shifts used for 16b fixed point conversion. Perform shift before adding bias.
	FIX_MAP *p_output	// pointer to output feature maps. Stored in [map][row][col] flattened manner.
	);

STATUS_E dsp_fix_conv_3x3(FIX_MAP *p_input,	// pointer to input maps stored in flattened [maps][row][col] format.
	FIX_KER *p_weight,	// pointer to kernels stored in flattened [no_outputs][no_inputs][ker_size][ker_size] format
	FIX_KER *p_bias,	// pointer to bias units. there are 'no_outputs' bias units
	int in_height,		// input feature map height
	int in_width,		// input feature map width
	int no_inputs,		// number of input feature maps
	int no_outputs,		// number of output feature maps
	int start_map,		// map offset to start fot this core.
	int no_maps, 		// no of feature maps assigned to this core.
	int pad,			// padding in all 4 sides. Only equal padding in all dimensions is supported.
	int stride,			// convolution window stride in both horizontal and vertical direction.
	int shift,			// Shifts used for 16b fixed point conversion. Perform shift before adding bias.
	FIX_MAP *p_output	// pointer to output feature maps. Stored in [map][row][col] flattened manner.
	);

// Use this when pad == 0 AND in_width % 2 == 0 AND weights are stored in on-chip memory
STATUS_E dsp_fix_conv_3x3_constrained(FIX_MAP *p_input,	// pointer to input maps stored in flattened [maps][row][col] format.
	FIX_KER *p_weight,	// pointer to kernels stored in flattened [no_outputs][no_inputs][ker_size][ker_size] format
	FIX_KER *p_bias,	// pointer to bias units. there are 'no_outputs' bias units
	int in_height,		// input feature map height
	int in_width,		// input feature map width
	int no_inputs,		// number of input feature maps
	int no_outputs,		// number of output feature maps
	int start_map,		// map offset to start fot this core.
	int no_maps, 		// no of feature maps assigned to this core.
	int stride,			// convolution window stride in both horizontal and vertical direction.
	int shift,			// Shifts used for 16b fixed point conversion. Perform shift before adding bias.
	FIX_MAP *p_output	// pointer to output feature maps. Stored in [map][row][col] flattened manner.
	);
STATUS_E dsp_conv_layer(CONV_LYR_CTX_T *p_conv_ctx, FLT_MAP *p_flt_in_maps, FIX_MAP *p_fix_in_map);
#endif // _CONV_LAYER_H
