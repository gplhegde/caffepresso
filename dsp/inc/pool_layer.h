#ifndef _POOL_LAYER_H
#define _POOL_LAYER_H
#include "struct_defs.h"
#include "caffe_proto_params.h"
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
	// pooling window size.
	int win_size;
	// pooling stride. Currently horz stride == vertical stride.
	int stride;
	// padding on all 4 sides of the input feature maps.
	int pad;
	// type of pooling.
	POOL_TYPE_E pool_type;
} POOL_INFO_T;

//=================================================
//=================================================
// Pooling layer context
typedef struct {
	// Floating point output map buffer for this layer.
	FLT_MAP *p_flt_output;
	// Fixed point output map buffer for this layer.
	FIX_MAP *p_fix_output;
	// Arithmetic mode to be used to compute the output. float/fixed point
	LYR_ARITH_MODE_E lyr_arith_mode;
	// starting output map assigned to this core. this number is zero based
	uint16_t start_map[NO_CORES];
	// number of output maps assigned to this core.
	uint16_t no_maps[NO_CORES];
	// Pooling layer parameters
	POOL_INFO_T pool_info;
} POOL_LYR_CTX_T;
//=================================================
//=================================================


STATUS_E dsp_pool_layer(POOL_LYR_CTX_T *p_pool_ctx, FLT_MAP *p_flt_in_maps, FIX_MAP *p_fix_in_maps);
#endif // _POOL_LAYER_H
