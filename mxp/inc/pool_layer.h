#ifndef _POOL_LAYER_H_
#define _POOL_LAYER_H_
#include "struct_defs.h"
#include "caffe_frontend.h"
//=================================================
//=================================================
// Pooling layer parameter structure.
typedef struct {
	int mapH;
	int mapW;
	int nMaps;
	int winSize;
	int stride;
	int pad;
	POOL_TYPE_E poolType;
} POOL_INFO_T;

//=================================================
//=================================================
// Pooling layer context
typedef struct {
	// Floating point output map buffer for this layer.
	FL_MAP_PIXEL *pFloatOutput;
	// Fixed point output map buffer for this layer.
	FP_MAP_PIXEL *pFixOutput;
	// Map layout
	MAP_LAYOUT_E mapLyt;
	// Arithmetic mode to be used to compute the output. float/fixed point
	LYR_ARITH_MODE_E lyrArithMode;
	// Layer optimization mode
	OPT_TYPE_E optType;
	// Pooling layer parameters
	POOL_INFO_T poolInfo;
} POOL_LYR_CTX_T;

//=================================================
//=================================================
/* \brief Abstract pooling API. This supports float/fixed point and scalar/vector implementations.
*
* @param pPoolCtx       : Pooling layer context containing all necessary information of a pooling layer.
* @param pFloatInMaps   : Floating point input maps.
* @param pFixInMaps     : Fixed point input maps.
* @param inMapLyt       : Input map storage layout
* @return               : Pooling layer status.
*/
APP_STATUS_E cnn_pool_layer(POOL_LYR_CTX_T *pPoolCtx,
	FL_MAP_PIXEL *pFloatInMaps,
	FP_MAP_PIXEL *pFixInMaps,
	MAP_LAYOUT_E inMapLyt);
#endif // _POOL_LAYER_H_
