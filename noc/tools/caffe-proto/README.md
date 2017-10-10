#Description
This directory contains tools to generate source and header files for network definition parameters and network weights.
- **parse-proto**: contains scripts to parse .prototxt files containing network definition in the Caffe format. This tool will generates a C source and header file which need to be copied over to the main project src and inc directories respectively.
- **parse-lmdb**: contains tools to parse trained network model in LMDB format and generate a C source and header files containing network weights. These generated files need to be copied over to the main project **src** and **inc** directories respectively.

Please refer to the instructions inside each of these tools directories to use them.
#Notice
./caffe.proto and ./gen-header/caffe_pb2.py are under the LICENSE file present in this directory.
