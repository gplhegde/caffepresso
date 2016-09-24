#ifndef _CONV_LAYER_H_
#define _CONV_LAYER_H_

#include "struct_defs.h"

//=============================================================
//=============================================================
// Convolution layer parameter info
typedef struct {
	// Input map height
	int mapH;
	// Input map width
	int mapW;
	// Kernel size of the conv layer
	int K;
	// No of input maps to this layer
	int nInMaps;
	// No of output maps produced by this layer
	int nOutMaps;
	// horizontal and vertical pad
	int pad;
	// horizontal and vertical pad
	int stride;
	// No of fraction bits in kernel coefficient representation
	int nKerFractionBits;
	// No of fraction bits in map pixel representation
	int nMapFractionBits;
} CONV_INFO_T;
//=============================================================
//=============================================================
// Patch based implementation related info.
typedef struct {
	// Optimum block height for this layer
	int blkH;
	// Optimum block width for this layer
	int blkW;
	// Optimum no of maps to be computed at once for this layer
	int optMaps;
} BLK_INFO_T;
//=============================================================
//=============================================================
// Convolution layer context
typedef struct {
	// Floating point output map buffer for this layer.
	FL_MAP_PIXEL *pFloatOutput;
	// Fixed point output map buffer for this layer.
	FP_MAP_PIXEL *pFixOutput;
	// Floating point kernels
	FL_KERNEL *pFloatKer;
	// Fixed point kernel buffer
	FP_KERNEL *pFixKer;
	// Fixed point extended kernel buffer
	FP_KERNEL **ppExtKer;
	// Floating point bias buffer
	FL_KERNEL *pFloatBias;
	// Fixed point bias buffer
	FP_KERNEL *pFixBias;
	// Map layout
	MAP_LAYOUT_E mapLyt;
	// Layer parameters.
	CONV_INFO_T convInfo;
	// Arithmetic mode to be used to compute the output. float/fixed point
	LYR_ARITH_MODE_E lyrArithMode;
	// Layer optimization mode
	OPT_TYPE_E optType;
	// Patch based optimum parameters
	BLK_INFO_T blkInfo;
	
} CONV_LYR_CTX_T;
//=============================================================
//=============================================================

/* \brief A generic convolution layer API supporting float/fixed point and scalar/vector inplementation.
*
* @param pConvCtx       : Convolution layer context.
* @param pFloatInMaps   : Input feature maps in floating point.
* @param pInFixMaps     : Input feature maps in fixed point format.
* @param inMapLyt       : Input feature map storage layout. isolated/concatenated.
* @return               : status of conv layer computation
*/
APP_STATUS_E cnn_conv_layer(CONV_LYR_CTX_T *pConvCtx,
	FL_MAP_PIXEL *pFloatInMaps,
	FP_MAP_PIXEL *pFixInMaps,
	MAP_LAYOUT_E inMapLyt);
#endif // _CONV_LAYER_H_
