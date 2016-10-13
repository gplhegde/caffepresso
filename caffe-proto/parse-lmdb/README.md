#Description
This tool will generate C source files from the network definition .prototxt file and the trained model in the .caffemodel LMDB format.

#Dependencies
1. Caffe : See installation instructions [here](http://caffe.berkeleyvision.org/installation.html)
2. Python 2.7 packages: Numpy, argparse, termcolor

#How to run?
Once you installed all dependencies, run follow command to generate C source files

*python lmdb_parser.py --model=`<network-model-prototxt`> --lmdb=`<trained-model-in-caffemodel-format`>*

The tool will generate two files, network_model.c and network_model.h. Copy the C file to **src** directory of the main project and the header file to **inc** directory.
