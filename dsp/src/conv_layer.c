#include "conv_layer.h"
#include "struct_defs.h"

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
	) {

	STATUS_E status = FAILED;
	//TODO
	// Refer to https://github.com/gplhegde/caffepresso/blob/master/mxp/src/conv_layer.c:scalar_fix_conv_layer function
	// for implementation details.
	return status;
}

