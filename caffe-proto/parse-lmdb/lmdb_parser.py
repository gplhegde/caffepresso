import sys, os
import numpy as np
import argparse
from termcolor import colored, cprint
from lmdb_utils import dump_conv_weights, dump_ip_weights 

model_src_file = 'network_model.c'
model_inc_file = 'network_model.h'

def parse_args():
    """Argument parser for this tool
    """
    parser = argparse.ArgumentParser(description='LMDB model to source file generator.')
    parser.add_argument('--model', dest='model_file', help='Network definition file in .prototxt format.')
    parser.add_argument('--lmdb', dest='lmdb_file', help='Network trained model in LMDB format.')

    # parse command line args
    if(len(sys.argv) < 3):
        parser.print_help()
        sys.exit()
    args = parser.parse_args()
    return args

def dump_model(net_def_file, model_file):

    c_file_header = ('/* Network model  weights and biases*/\n'
            '#include "' + model_inc_file + '"\n\n')

    h_file_header = ('/*Header file for model  weights and biases*/\n'
        '#ifndef _NETWORK_MODEL_H_\n#define _NETWORK_MODEL_H_\n#include <stdio.h>\n\n')

    h_file = open(model_inc_file, 'w')
    c_file = open(model_src_file, 'w')
    h_file.write(h_file_header)
    c_file.write(c_file_header)
    h_file.close()
    c_file.close()

    # init the network with the trained model
    caffe.set_mode_cpu()    
    net = caffe.Net(net_def_file, model_file, caffe.TEST)

    param_layers = net.params.keys()
    lyr_name = 0
    for lyr in net.layers:
        if(lyr.type == 'Convolution'):
            dump_conv_weights(net, param_layers[lyr_name], model_src_file, model_inc_file)
            lyr_name += 1
        elif(lyr.type == "InnerProduct"):
            dump_ip_weights(net, param_layers[lyr_name], model_src_file, model_inc_file)
            lyr_name += 1
        elif(lyr.type == "BatchNorm"):
            lyr_name += 1
            print('We do not support batch normalization layer yet')

    h_file = open(model_inc_file, 'a')
    h_file.write('#endif // _NETWORK_MODEL_H_')
    h_file.close()

    cprint('Generated source files successfully', 'green')
    cprint('Copy {:s} and {:s} to the main project src and inc directories respectively.'.format(model_src_file, model_inc_file), 'green')

if __name__=='__main__':
    args = parse_args()

    # make sure that path to caffe root directory is set
    assert ('CAFFE_ROOT' in os.environ) , ('Please set CAFFE_ROOT in the environment'
        'variable to point to the caffe installation directory')

    # import caffe package
    caffe_pkg_path = os.path.join(os.environ['CAFFE_ROOT'], 'python')
    sys.path.insert(0, caffe_pkg_path)
    import caffe


    dump_model(args.model_file, args.lmdb_file)

