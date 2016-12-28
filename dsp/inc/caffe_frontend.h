#ifndef _CAFFE_FRONTEND_H_
#define _CAFFE_FRONTEND_H_

#include "caffe_proto_params.h"

typedef struct {
	
	CNN_LAYER_TYPE_E lyr_type;
	void *p_lyr_ctx;
} CNN_LYR_NODE_T;

extern CNN_LYR_NODE_T g_cnn_layer_nodes[NO_DEEP_LAYERS];

void caffe_layer_ctx_init();

unsigned long  cnn_layer_internal_param_init(void);
#endif // _CAFFE_FRONTEND_H_
