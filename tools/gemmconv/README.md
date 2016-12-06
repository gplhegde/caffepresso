# Description
This tool will generate matrix dimensions for GEMM based convolution of all convolution layers present in the given network model file(.prototxt). These dimensions are used to get the runtime of different convolution layers using GEMM based implementation and compare with the conventional implementation.

# To run
**python gemm_size_estimate.py --model=**`<prototxt file`> **--prefix=**`<prefix that is used for naming variables(optional)`>
Ex:
*python gemm_size_estimate.py --model=../caffe-proto/parse-lmdb/lenet.prototxt --prefix=lenet*

You can specify the linker attributes for the arrays created by updating **linker_attr** variable in the script.