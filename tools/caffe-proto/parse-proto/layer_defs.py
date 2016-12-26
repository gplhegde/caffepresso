
LayerEnums = ("CONV", "POOL", "ACT", "BATCH_NORM", "INNER_PROD", "SOFTMAX", "UNSUPPORTED_LYR")
PoolTypeEnums = ("MAX_POOL", "AVG_POOL", "UNSUPPORTED_POOL")
ActivationEmums = ("RELU", "SIGMOID", "TANH", "UNSUPPORTED_ACT")

LayerParamContainer = {
	"topCon"	: None,
	"botCon"	: None,
	"lyrName"	: None,
	"lyrType"	: ["UNSUPPORTED_LYR", "CNN_LAYER_TYPE_E"],
	"K"			: [-1, "int"],
	"nOutMaps"	: [-1, "int"],
	"winSize"	: [-1, "int"],
	"stride"	: [-1, "int"],
	"pad"		: [0, "int"],
	"poolType"	: [-1, "POOL_TYPE_E"],
	"actType"	: [-1, "ACT_TYPE_E"],
	"nOutputs"	: [-1, "int"]
} 
