#ifndef _MERGED_LAYERS_H_
#define _MERGED_LAYERS_H_

// Convolution + Pool layer context
typedef struct {
	// Floating point output map buffer for this layer.
	FL_MAP_PIXEL *pFloatOutput;
	// Floating point kernels
	FL_KERNEL *pFloatKer;
	// Floating point bias buffer
	FL_KERNEL *pFloatBias;
	// Map layout
	MAP_LAYOUT_E mapLyt;
	// Layer parameters.
	CONV_INFO_T convInfo;
	// Layer optimization mode
	OPT_TYPE_E optType;
	// Patch based optimum parameters
	BLK_INFO_T blkInfo;
	// Pooling layer parameters
	POOL_INFO_T poolInfo;
	
} CONV_POOL_LYR_CTX_T;

#endif // _MERGED_LAYERS_H_