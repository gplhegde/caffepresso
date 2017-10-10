#Current Status - PLEASE READ
The Epiphany open-source release is under heavy development for full-functionality support. If you have your network protobuf description, and an LMDB database file that contains your network weight parameters, you can evaluate your network on a new image using only the ARM CPU on the Parallella board. Support for the ARM+Epiphany implementation is being reworked at the moment for faster evaluation and tighter Caffe integration. If you would like to contribute, please contact the author (sidmontu AT gmail DOT com).

#Description
Implementation of deep learning convolutional neural networks (CNNs) for the Parallella Epiphany accelerator. This repository contains the following nets implemented on the Parallella board:

- MNIST
- CIFAR10
- CALTECH101
- STL10
- AlexNet

# Small networks
MNIST and CIFAR10 are fairly small networks and hence, their implementation is designed by hand since all intermediate data can fit on the on-chip memory on the Epiphany accelerator. The implementations can be found in net/mnist and net/cifar10 directories respectively. To compile either MNIST or CIFAR10, cd to respective directory, and type `./build.sh`, followed by `./run.sh <num_timesteps>` to run each model.

#Patch-based approach for larger networks
The larger networks need to be broken up into stages as they are too large to fit onto the Epiphany accelerator for a single-shot evaluation, unlike MNIST and CIFAR10. We develop a general patch-based compute strategy for tackling these larger networks, and develop tools around this approach to make a feasible mapping for the Epiphany.

Larger networks are evaluated one layer at a time, where each layer is evaluated in parallel across all the 16 eCores of the Epiphany accelerator. The output maps from each layer are constructed in a fragmented manner, one patch at a time. The challenge is to achieve functional correctness due to ghost-pixel regions that are needed from convolution operations. We use both the ARM and the Epiphany to evaluate these larger networks, and use careful unrolling strategies on the ARM such that all 16 eCores on the Epiphany can issue independent read/write requests to the off-chip DRAM without conflicts or data hazards. In essence, the Parallella mapping for deep learning networks is a map-parallel appraoch, where the acceleration comes from evaluating multiple maps (and patches) in parallel across all 16 Epiphany eCores.

#Compilation and Running
To compile and run these networks, we need a Parallella board with an Epiphany 16-eCore accelerator chip (not tested with the 64-eCore variant). PAPI timing routines can be enabled/disabled for profiling the performance (tool used for testing and reporting the numbers in the paper).

**To run a network on the Parallella**

The Parallella eSDK must be installed on the host machine for the source to compile. To run the mapping, the host machine must be a Parallella board with the Epiphany 16-eCore accelerator. The following steps must be first taken before issuing a _make_ command.

1. Use the caffe-proto python tool inside tools/caffe-proto/parse-proto to generate caffe_proto_params.c and caffe_proto_params.h files, which contain data structures that describe your network in C.
2. Use the caffe-lmdb python tool inside tools/caffe-proto/parse-lmdb to generate network_model.c and network_model.h files, which contain weights trained in your model.
3. Copy the above-generated .c files into src/ folder and .h files into include/ folder.
4. Type `make`
5. Type `make run` to run inference on a test/input image.

