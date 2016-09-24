#ifndef _CAFFE_FRONTEND_H_
#define _CAFFE_FRONTEND_H_

#include "caffe_proto_params.h"

typedef struct {
	
	CNN_LAYER_TYPE_E lyrType;
	void *pLyrCtx;	
} CNN_LYR_NODE_T;

extern CNN_LYR_NODE_T cnnLayerNodes[NO_DEEP_LAYERS];

void caffe_layer_ctx_init();

void cnn_layer_internal_param_init(void);
#endif // _CAFFE_FRONTEND_H_
