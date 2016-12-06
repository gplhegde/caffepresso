import sys
import caffe_pb2
import argparse
from copy import deepcopy
import collections
from google.protobuf.text_format import Merge
from termcolor import cprint
from layer_defs import LayerParamContainer, LayerEnums, ActivationEmums, PoolTypeEnums

headerFileName = "caffe_proto_params.h"
sourceFileName = "caffe_proto_params.c"
startString = "#ifndef _CAFFE_PROTO_PARAMS_H_\n#define _CAFFE_PROTO_PARAMS_H_\n\n"
endString = "#endif // _CAFFE_PROTO_PARAMS_H_"
docString = "/*This header is automatically generated from caffe prototxt format.\nThe variable names influence of majority of APIs in the C application\n" \
	"Do not modify this file*/\n"

arrayName = "cnn_param_table"
structName = "CAFFE_LYR_PARAM_T"
NumCnnLayers = 0

class DnnHeaderCreater(object):
	def __init__(self, net, hfile, sfile):
		self.net = net
		self.hfile = hfile
		self.sfile = sfile
		self.noInputMaps = 0
		self.inputWidth  = 0
		self.inputHeight = 0
		self.inputTopCon = ""
		self.strArray = []
		self.orderedStructs = []
	def update_input_lyr_param(self, lyr):
		self.noInputMaps = lyr.input_param.shape[0].dim[1]
		self.inputHeight = lyr.input_param.shape[0].dim[2]
		self.inputWidth = lyr.input_param.shape[0].dim[3]
		self.inputTopCon = str(lyr.top[0])

	def update_conv_lyr_struct(self, lyr):
		print("Updating conv layer...")
		convParam = deepcopy(LayerParamContainer)
		convParam["lyrType"][0] 	= "CONV"
		convParam["K"][0] 			= lyr.convolution_param.kernel_size[0]
		convParam["nOutMaps"][0] 	= lyr.convolution_param.num_output
		if len(lyr.convolution_param.stride) != 0:
			convParam["stride"][0] 		= lyr.convolution_param.stride[0]
		else:
			convParam["stride"][0] 		= 1
		if(lyr.convolution_param.pad):
			convParam["pad"][0] = lyr.convolution_param.pad[0]
		convParam["topCon"] 		= str(lyr.top[0])
		convParam["botCon"]			= str(lyr.bottom[0])
		convParam["lyrName"]		= lyr.name
		self.strArray.append(convParam)

	def update_pool_lyr_struct(self, lyr):
		print("Updating pooling layer...")
		poolParam = deepcopy(LayerParamContainer)
		poolParam["lyrType"][0] 	= "POOL"
		poolParam["stride"][0]		= lyr.pooling_param.stride
		poolParam["winSize"][0]		= lyr.pooling_param.kernel_size
		poolParam["poolType"][0] 	= lyr.pooling_param.pool
		poolParam["pad"][0]			= lyr.pooling_param.pad
		poolParam["topCon"] 		= str(lyr.top[0])
		poolParam["botCon"]			= str(lyr.bottom[0])
		poolParam["lyrName"]		= lyr.name
		self.strArray.append(poolParam)

	def update_act_lyr_struct(self, lyr):
		actParam = deepcopy(LayerParamContainer)
		actParam["lyrType"][0] 		= "ACT"
		actParam["actType"][0] 		= str(lyr.type).upper()
		actParam["topCon"] 			= str(lyr.top[0])
		actParam["botCon"]			= str(lyr.bottom[0])
		actParam["lyrName"]		= lyr.name
		self.strArray.append(actParam)
		

	def update_ip_lyr_struct(self, lyr):
		ipParam = deepcopy(LayerParamContainer)
		ipParam["lyrType"][0] 		= "INNER_PROD"
		ipParam["nOutputs"][0] 		= lyr.inner_product_param.num_output
		ipParam["topCon"] 			= str(lyr.top[0])
		ipParam["botCon"]			= str(lyr.bottom[0])
		ipParam["lyrName"]		= lyr.name
		self.strArray.append(ipParam)

	def update_smax_lyr_struct(self, lyr):
		smaxParam = deepcopy(LayerParamContainer)
		smaxParam["lyrType"][0] 	= "SOFTMAX"
		smaxParam["topCon"] 		= str(lyr.top[0])
		smaxParam["botCon"]			= str(lyr.bottom[0])
		smaxParam["lyrName"]		= lyr.name
		self.strArray.append(smaxParam)
		
	
	def parse_params(self):
		for lyr in self.net.layer:
			if lyr.type == "Input":
				self.update_input_lyr_param(lyr)
			elif lyr.type == "Convolution":
				self.update_conv_lyr_struct(lyr)
			elif lyr.type == "Pooling":
				self.update_pool_lyr_struct(lyr)
			elif lyr.type == "InnerProduct":
				self.update_ip_lyr_struct(lyr)
			elif lyr.type == "Softmax":
				self.update_smax_lyr_struct(lyr)
			elif lyr.type == "ReLU" or lyr.type == "Sigmoid" or lyr.type == "TanH":
				self.update_act_lyr_struct(lyr)
			else:
				raise ValueError(lyr.type + "This layer is not supported as of now.")

	def order_structures(self):
		print("Re-ordering the list elements if required")
		topCon = [e["topCon"] for e in self.strArray]
		botCon = [e["botCon"] for e in self.strArray]
		# check if the output of each layer is connected to exactly one layer.
		# we suport only such connections as of now.
		topC = list(collections.Counter(topCon).items())
		botC = list(collections.Counter(botCon).items())
		#print(topC)
		#print(botC)
		
		for t, b in zip(topC, botC):
			if t[0] == b[0]:
				if t[1] != b[1]:
					raise ValueError("Bottom of some layer is connected to more than 1 layer. Not supported as of now")		

		topLyrIdx = 0
		doneStruct = []
		for lyr in self.strArray:
			if lyr["botCon"] == self.inputTopCon:
				topLyrIndx = self.strArray.index(lyr)
				# store the reference to the layer whose input is connected to data
				# This will be the first layer
				self.orderedStructs.append(lyr)
				doneStruct.append(topLyrIdx)

		# Append all layer structs but layer 0
		while len(self.orderedStructs) < len(self.strArray):
			for lyr in self.strArray:
				curIdx = self.strArray.index(lyr)
				if curIdx not in doneStruct:
					if self.orderedStructs[-1]["topCon"] == lyr["botCon"]:
						self.orderedStructs.append(lyr)
						doneStruct.append(curIdx)
			
	def print_container(self):
		print("Input no of maps = " + str(self.noInputMaps))
		print("Input height = " + str(self.inputHeight))
		print("Input width = " + str(self.inputWidth))
		print("Input's top connection = " + self.inputTopCon)
		print("List of struct as parsed from the prototxt")
		for e in self.strArray:
			print(e)
		print("Ordered list as per connections")
		for e in self.orderedStructs:
			print(e)

	def write_struct_definition(self, name):

		# layer type enum
		self.hfile.write("\n/*Supported CNN layers*/\ntypedef enum {\n")
		for e in LayerEnums:
			self.hfile.write("\t" + e)
			if e == LayerEnums[-1]:
				self.hfile.write("\n} CNN_LAYER_TYPE_E;\n\n")
			else:
				self.hfile.write(",\n")
		# pool type enum
		self.hfile.write("\n/*Supported Pooling types*/\ntypedef enum {\n")
		for e in PoolTypeEnums:
			self.hfile.write("\t" + e)
			if e == PoolTypeEnums[-1]:
				self.hfile.write("\n} POOL_TYPE_E;\n\n")
			else:
				self.hfile.write(",\n")
		
		# activation type enum
		self.hfile.write("\n/*Supported activation types*/\ntypedef enum {\n")
		for e in ActivationEmums:
			self.hfile.write("\t" + e)
			if e == ActivationEmums[-1]:
				self.hfile.write("\n} ACT_TYPE_E;\n\n")
			else:
				self.hfile.write(",\n")
		# layer parameter structure 
		self.hfile.write("/*CNN layer parameter structure*/")
		self.hfile.write("\ntypedef struct {\n")
		for key in LayerParamContainer:
			if key == "lyrName" or key == "topCon" or key == "botCon":
				continue
			else:
				self.hfile.write("\t" + LayerParamContainer[key][1] + " " + key + ";\n")
		self.hfile.write("} " + name + ";\n\n")

	def write_struct_array(self, arrayType):
		self.sfile.write("\n/* Array of structure containing CNN parameters. Each structure represents 1 layer.\
		\nThe order of the layers follow this structure.*/\n\n")
		self.sfile.write("const " + arrayType + " " + arrayName + "[NO_DEEP_LAYERS] = {\n")
		def write_struct(lyr, last):
			self.sfile.write("{")
			self.sfile.write(".lyrType = " + str(lyr["lyrType"][0]) + ",\n")
			self.sfile.write(".K = " + str(lyr["K"][0]) + ",\n")
			self.sfile.write(".nOutMaps = " + str(lyr["nOutMaps"][0]) + ",\n")
			self.sfile.write(".winSize = " + str(lyr["winSize"][0]) + ",\n")
			self.sfile.write(".stride = " + str(lyr["stride"][0]) + ",\n")
			self.sfile.write(".pad = " + str(lyr["pad"][0]) + ",\n")
			self.sfile.write(".poolType = " + str(PoolTypeEnums[lyr["poolType"][0]]) + ",\n")
			self.sfile.write(".actType = " + str(lyr["actType"][0]) + ",\n")
			self.sfile.write(".nOutputs = " + str(lyr["nOutputs"][0]) + ",\n")
			if last == False:
				self.sfile.write("},\n")
			else:
				self.sfile.write("}\n")

		size = len(self.orderedStructs)
		last = False
		for lyr in self.orderedStructs:
			if lyr == self.orderedStructs[-1]:
				last = True
			write_struct(lyr, last)

		self.sfile.write("};\n\n")

	def compute_no_ops(self):
		sp_size = 128*1024
		import math
		def conv_ops(in_width, in_height, no_input, no_output, k, stride):
			print 'conv:', in_width, in_height, no_input, no_output, k, stride
			ops = in_width * in_height * no_input * no_output * 2 * k * k
			ops += in_width * in_height * no_input * no_output
			ops = ops / (stride ** 2)
			bw = in_width
			no_blks = 1
			rd_ops = no_input* no_blks* (2*in_height*bw + bw*in_height*2 + in_height*k*k*bw*no_output*2) + \
			    no_input*(in_width-k+1)*(in_height-k+1)*no_output*2 + (in_width-k+1)*(in_height-k+1)*no_output*4
			wr_ops = no_input*no_blks*(2*no_output*(in_height-k+1)*(in_height-k+1) + bw*in_height*no_output*2) \
			    + 4*no_input*(in_height-k+1)*(in_width-k+1)*no_output + 2*no_output*(in_height-k+1)*(in_width-k+1)
			print rd_ops
			return ops, int(rd_ops), int(wr_ops)

		def pool_ops(in_width, in_height, win_size, stride, n_maps):
			print 'pool:', in_width, in_height, win_size, stride, n_maps
			ops = in_width * in_height * (win_size * win_size - 1)
			ops = ops * n_maps
			ops = ops / (stride ** 2)
			rd_ops = n_maps * in_height * in_width *2
			wr_ops = (n_maps * in_height * in_width * 2)/(stride**2)
			print rd_ops
			return ops, int(rd_ops), int(wr_ops)

		def relu_ops(in_width, in_height, n_maps):
			print 'relu:', in_width, in_height, n_maps
			ops = in_width * in_height * n_maps
			rd_ops = n_maps * in_width* in_height * 2
			wr_ops = n_maps * in_width* in_height * 2
			print rd_ops
			return ops, rd_ops, wr_ops
		def fc_ops(no_input, no_output):
			print 'fc', no_input, no_output
			ops = 2 * no_input * no_output
			rd_ops = no_input * no_output* 2* 8
			wr_ops = no_output * 4
			print rd_ops
			return ops, int(rd_ops), int(wr_ops)

		print self.noInputMaps
		print self.inputHeight
		print self.inputWidth
		no_in_maps = self.noInputMaps
		map_width = self.inputWidth
		map_height = self.inputHeight
		total_ops = 0
		total_rd_ops = 0
		total_wr_ops = 0
		for lyr in self.orderedStructs:
			if lyr['lyrType'][0] == 'CONV':
				arith_ops, rd_ops, wr_ops = conv_ops(map_width, map_height, no_in_maps, lyr['nOutMaps'][0], lyr['K'][0], lyr['stride'][0])
				total_ops += arith_ops
				total_rd_ops += rd_ops
				total_wr_ops += wr_ops
				no_in_maps = lyr['nOutMaps'][0]
				map_width = int((map_width-lyr['K'][0]+1)/ lyr['stride'][0])
				map_height = int((map_height-lyr['K'][0]+1) / lyr['stride'][0])
			elif lyr['lyrType'][0] == 'POOL':
				arith_ops, rd_ops, wr_ops = pool_ops(map_width, map_height, lyr['winSize'][0], lyr['stride'][0], no_in_maps)
				total_ops += arith_ops
				total_rd_ops += rd_ops
				total_wr_ops += wr_ops

				map_width /= lyr['stride'][0]
				map_height /= lyr['stride'][0]
				map_height = max(1, map_height)
				map_width = max(1, map_width)
			elif lyr['lyrType'][0] == 'INNER_PROD':
				arith_ops, rd_ops, wr_ops = fc_ops(map_height * map_width * no_in_maps, lyr['nOutputs'][0])
				total_ops += arith_ops
				total_rd_ops += rd_ops
				total_wr_ops += wr_ops
				map_height = 1
				map_width = 1
				no_in_maps = lyr['nOutputs'][0]
			elif lyr['lyrType'][0] == 'ACT':
				arith_ops, rd_ops, wr_ops = relu_ops(map_width, map_height, no_in_maps)
				total_ops += arith_ops
				total_rd_ops += rd_ops
				total_wr_ops += wr_ops
			else:
				print('No of ops undefined for this layer')
		print('total arithmetic operations = ' + str(total_ops))
		print('total read ops = {:d}'.format(total_rd_ops))
		print('total write ops = {:d}'.format(total_wr_ops))

def create_header(prototxtFile):
	# Create the Caffe network parameter object
	net = caffe_pb2.NetParameter()

	# merge the given network with the object. This will read the text file and initialize all
	# parameters of the object with those from the text file.
	Merge((open(prototxtFile, 'r').read()), net)

	hfile = open(headerFileName, 'w')
	sfile = open(sourceFileName, 'w')
	hfile.write(startString)
	hfile.write(docString)
	sfile.write(docString)
	sfile.write("#include " + '"' + headerFileName + '"'+ "\n\n")

	# find the total number of layers in the network. Input layer is not counted as it is a data layer.
	# hence -1 
	NumCnnLayers = len(net.layer)
	hfile.write("#define NO_DEEP_LAYERS " + str(NumCnnLayers-1) + "\n")

	# create header file writer object
	hw = DnnHeaderCreater(net, hfile, sfile)

	hw.parse_params()

	# make sure the structures are arranged in the layer connection format
	hw.order_structures()
	hfile.write("#define INPUT_IMG_WIDTH " + str(hw.inputWidth) + "\n")
	hfile.write("#define INPUT_IMG_HEIGHT " + str(hw.inputHeight) + "\n")
	hfile.write("#define NO_INPUT_MAPS " + str(hw.noInputMaps) + "\n")

	# write general struct definition to the header file
	hw.write_struct_definition(structName)

	# declare the array of structure
	hfile.write("extern " + "const " + structName + " " + arrayName + "[NO_DEEP_LAYERS];\n\n")
	hw.write_struct_array(structName)

	# write the header end string
	hfile.write(endString)

    # print approx no of operations present in the network
	#hw.compute_no_ops()

	hfile.close()
	sfile.close()
	cprint('Generated source files successfully', 'green')
	cprint('Copy {:s} and {:s} to the main project src and inc directories respectively.'.format(sourceFileName, headerFileName), 'green')

def parse_args():
    """Argument parser for this tool
    """
    parser = argparse.ArgumentParser(description='LMDB model to source file generator.')
    parser.add_argument('--model', dest='model_file', help='Network definition file in .prototxt format.')

    # parse command line args
    if(len(sys.argv) < 2):
        parser.print_help()
        sys.exit()
    args = parser.parse_args()
    return args

if __name__ == '__main__':

	# parse the arguments
	args = parse_args()

	create_header(args.model_file)
