#ifndef _ACTIVATION_LAYER_H
#define _ACTIVATION_LAYER_H
#include "struct_defs.h"
#include "caffe_proto_params.h"
#include "user_config.h"

//========================================
//========================================
// Activation layer parameters.
typedef struct {
	int map_h;
	int map_w;
	// No of input neurons/maps
	int no_inputs;
	// No of output neurons/maps
	int no_outputs;
	ACT_TYPE_E act_type;
} ACT_INFO_T;

//========================================
//========================================
// Activation layer context
typedef struct {
	// Floating point output map buffer for this layer.
	FLT_MAP *p_flt_output;
	// Fixed point output map buffer for this layer.
	FIX_MAP *p_fix_output;
	// Arithmetic mode to be used to compute the output. float/fixed point
	LYR_ARITH_MODE_E lyr_arith_mode;
	// start map/node
	uint16_t start_map[NO_CORES];
	// number of assigned maps or output nodes assigned to this core.
	uint16_t no_maps[NO_CORES];
	// Parameters
	ACT_INFO_T act_info;

} ACT_LYR_CTX_T;

/* \brief Abstract activation layer supporting float/fixed point and scalar/vector operations
*
* @param p_act_ctx        : Activation layer context. Refer to activation_layer.h for structure info.
* @param p_flt_in_maps   : Floating point input feature maps.
* @param p_fix_in_maps     : Input maps in the fixed point format.
* @return               : Activation layer status.
*/
STATUS_E dsp_activation_layer(ACT_LYR_CTX_T *p_act_ctx, FLT_MAP *p_flt_in_maps, FIX_MAP *p_fix_in_maps);
#endif // _ACTIVATION_LAYER_H
