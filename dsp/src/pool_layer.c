#include "pool_layer.h"
#include "struct_defs.h"

STATUS_E dsp_pool_layer(FIX_MAP *p_input,	// pointer to input maps stored in flattened [maps][row][col] format.
	int in_height,			// input feature map height
	int in_width,			// input feature map width
	int no_inputs,			// number of input feature maps
	int no_outputs,			// number of output feature maps
	int win_size,			// kernel size. We support only square sized kernels
	int stride,				// convolution window stride in both horizontal and vertical direction.
	POOL_TYPE_E pool_type,	// pooling type, max pooling or average pooling
	FIX_MAP *p_output		// pointer to output feature maps. Stored in [map][row][col] flattened manner.
	) {

	STATUS_E status = FAILED;
	//TODO
	// Refer to https://github.com/gplhegde/caffepresso/blob/master/mxp/src/pool_layer.c:scalar_fix_pool_layer function
	// for implementation details.
	return status;
}
