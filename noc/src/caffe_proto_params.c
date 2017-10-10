/*This header is automatically generated from caffe prototxt format.
The variable names influence of majority of APIs in the C application
Do not modify this file*/
#include "caffe_proto_params.h"


/* Array of structure containing CNN parameters. Each structure represents 1 layer.		
The order of the layers follow this structure.*/

const CAFFE_LYR_PARAM_T cnn_param_table[NO_DEEP_LAYERS] = {
{
    .lyrType = CONV,
    .K = 5,
    .nOutMaps = 20,
    .winSize = -1,
    .stride = 1,
    .pad = 0,
    .poolType = UNSUPPORTED_POOL,
    .actType = -1,
    .nOutputs = -1,
},
{
    .lyrType = POOL,
    .K = -1,
    .nOutMaps = -1,
    .winSize = 2,
    .stride = 2,
    .pad = 0,
    .poolType = MAX_POOL,
    .actType = -1,
    .nOutputs = -1,
},
{
    .lyrType = CONV,
    .K = 5,
    .nOutMaps = 50,
    .winSize = -1,
    .stride = 1,
    .pad = 0,
    .poolType = UNSUPPORTED_POOL,
    .actType = -1,
    .nOutputs = -1,
},
{
    .lyrType = POOL,
    .K = -1,
    .nOutMaps = -1,
    .winSize = 2,
    .stride = 2,
    .pad = 0,
    .poolType = MAX_POOL,
    .actType = -1,
    .nOutputs = -1,
},
{
    .lyrType = INNER_PROD,
    .K = -1,
    .nOutMaps = -1,
    .winSize = -1,
    .stride = -1,
    .pad = 0,
    .poolType = UNSUPPORTED_POOL,
    .actType = -1,
    .nOutputs = 500,
},
{
    .lyrType = ACT,
    .K = -1,
    .nOutMaps = -1,
    .winSize = -1,
    .stride = -1,
    .pad = 0,
    .poolType = UNSUPPORTED_POOL,
    .actType = RELU,
    .nOutputs = -1,
},
{
    .lyrType = INNER_PROD,
    .K = -1,
    .nOutMaps = -1,
    .winSize = -1,
    .stride = -1,
    .pad = 0,
    .poolType = UNSUPPORTED_POOL,
    .actType = -1,
    .nOutputs = 10,
},
{
    .lyrType = SOFTMAX,
    .K = -1,
    .nOutMaps = -1,
    .winSize = -1,
    .stride = -1,
    .pad = 0,
    .poolType = UNSUPPORTED_POOL,
    .actType = -1,
    .nOutputs = -1,
}
};

