#ifndef _APP_INIT_H_
#define _APP_INIT_H_
#include "struct_defs.h"
#include "cnn_layers.h"
#include "caffe_frontend.h"

STATUS_E init_conv_kernels(CONV_LYR_CTX_T *p_conv_ctx);

STATUS_E init_ip_layer_params(IP_LYR_CTX_T *p_ip_ctx);

STATUS_E cnn_app_malloc(CNN_LYR_NODE_T *pp_lyr_nodes, int no_layers, unsigned long  max_buff_req);
STATUS_E cnn_app_memfree();
STATUS_E cnn_app_model_init(CNN_LYR_NODE_T *pp_lyr_nodes, int no_layers);
STATUS_E workload_sharing_config(CNN_LYR_NODE_T *p_lyr_nodes, int n_layers);
#endif // _APP_INIT_H_
