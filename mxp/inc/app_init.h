#ifndef _APP_INIT_H_
#define _APP_INIT_H_
#include "struct_defs.h"
#include "cnn_layers.h"
//#include "caffe_"
APP_STATUS_E init_conv_kernels(CONV_LYR_CTX_T *pConvCtx);

APP_STATUS_E init_ip_layer_params(IP_LYR_CTX_T *pIpCtx);

APP_STATUS_E cnn_app_malloc(CNN_LYR_NODE_T *ppLyrNodes, int nLayers);
APP_STATUS_E cnn_app_memfree(CNN_LYR_NODE_T *ppLyrNodes, int nLayers);
APP_STATUS_E cnn_app_model_init(CNN_LYR_NODE_T *ppLyrNodes, int nLayers);
#endif // _APP_INIT_H_
