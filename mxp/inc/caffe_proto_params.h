#ifndef _CAFFE_PROTO_PARAMS_H_
#define _CAFFE_PROTO_PARAMS_H_

/*This header is automatically generated from caffe prototxt format.
The variable names influence of majority of APIs in the C application
Do not modify this file*/
#define NO_DEEP_LAYERS 8
#define INPUT_IMG_WIDTH 150
#define INPUT_IMG_HEIGHT 150
#define NO_INPUT_MAPS 1

/*Supported CNN layers*/
typedef enum {
	CONV,
	POOL,
	ACT,
	INNER_PROD,
	SOFTMAX,
	UNSUPPORTED_LYR
} CNN_LAYER_TYPE_E;


/*Supported Pooling types*/
typedef enum {
	MAX_POOL,
	AVG_POOL,
	UNSUPPORTED_POOL
} POOL_TYPE_E;


/*Supported activation types*/
typedef enum {
	RELU,
	SIGMOID,
	TANH,
	UNSUPPORTED_ACT
} ACT_TYPE_E;

/*CNN layer parameter structure*/
typedef struct {
	int K;
	int winSize;
	ACT_TYPE_E actType;
	int nOutMaps;
	POOL_TYPE_E poolType;
	int nOutputs;
	int stride;
	CNN_LAYER_TYPE_E lyrType;
} CAFFE_LYR_PARAM_T;

extern const CAFFE_LYR_PARAM_T cnn_param_table[NO_DEEP_LAYERS];

#endif // _CAFFE_PROTO_PARAMS_H_