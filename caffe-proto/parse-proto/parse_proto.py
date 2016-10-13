import caffe_pb2
from google.protobuf.text_format import Merge

net = caffe_pb2.NetParameter()
f = open("lenet.prototxt",'r')

Merge((open("lenet.prototxt",'r').read()), net)

def print_conv_param(conv_lyr):
	conv_param = {
		"name" 		: str(conv_lyr.name),
		"nOutMaps" 	: conv_lyr.convolution_param.num_output,
		"K" 		: conv_lyr.convolution_param.kernel_size[0]
	}
	print conv_param

def print_pool_param(pool_lyr):
	pool_param = {
		"name"		: str(pool_lyr.name),
		"win_size"	: pool_lyr.pooling_param.kernel_size,
		"stride"	: pool_lyr.pooling_param.stride
	}
	print pool_param

def print_ip_param(ip_lyr):
	ip_param = {
		"name"		: ip_lyr.name,
		"num_out"	: ip_lyr.inner_product_param.num_output
	}
	print ip_param

def print_act_param(act_lyr):
	act_param = {
		"name"	: act_lyr.name,
		"type"	: act_lyr.type
	}
	print act_param

def print_softmax_param(smax_lyr):
	softmax_param = {
		"name"	: smax_lyr.name	
	}
	print softmax_param

for lyr in net.layer:
	if lyr.type == "Input":
		for s in lyr.input_param.shape:
			print [s.dim[0], s.dim[1], s.dim[2], s.dim[3]]
	elif lyr.type == "Convolution":
		print_conv_param(lyr)
	elif lyr.type == "Pooling":
		print_pool_param(lyr)
	elif lyr.type == "InnerProduct":
		print_ip_param(lyr)
	elif lyr.type == "Softmax":
		print_softmax_param(lyr)
	elif lyr.type == "ReLU" or lyr.type == "Sigmoid" or lyr.type == "TanH":
		print_act_param(lyr)
	else:
		raise ValueError(lyr.type + "This layer is not supported as of now.")
		
