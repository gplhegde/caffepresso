#Description
Implementation of deep learning convolutional neural networks (CNNs) for the Parallella Epiphany accelerator. This repository contains the following nets implemented on the Parallella board:

- MNIST
- CIFAR10
- CALTECH101
- STL10
- AlexNet

# Small networks
MNIST and CIFAR10 are fairly small networks and hence, their implementation is designed by hand since all intermediate data can fit on the on-chip memory on the Epiphany accelerator. The implementations can be found in net/mnist and net/cifar10 directories respectively.

#Patch-based approach for larger networks
The larger networks need to be broken up into stages as they are too large to fit onto the Epiphany accelerator for a single-shot evaluation, unlike MNIST and CIFAR10. We develop a general patch-based compute strategy for tackling these larger networks, and develop tools around this approach to make a feasible mapping for the Epiphany.

Larger networks are evaluated one layer at a time, where each layer is evaluated in parallel across all the 16 eCores of the Epiphany accelerator. The output maps from each layer are constructed in a fragmented manner, one patch at a time. The challenge is to achieve functional correctness due to ghost-pixel regions that are needed from convolution operations. We use both the ARM and the Epiphany to evaluate these larger networks, and use careful unrolling strategies on the ARM such that all 16 eCores on the Epiphany can issue independent read/write requests to the off-chip DRAM without conflicts or data hazards. In essence, the Parallella mapping for deep learning networks is a map-parallel appraoch, where the acceleration comes from evaluating multiple maps (and patches) in parallel across all 16 Epiphany eCores.

#Compilation and Running
To compile and run these networks, we need a Parallella board with an Epiphany 16-eCore accelerator chip (not tested with the 64-eCore variant). For now, we have disabled measuring runtime, but PAPI timing routines can be inserted into the source code for profiling the performance (tested and reported in the paper). 

**To run a network on the Parallella**

The Parallella eSDK must be installed on the host machine for the source to compile. To run the mapping, the host machine must be a Parallella board with the Epiphany 16-eCore accelerator. The following steps must be first taken before issueing a _make_ command.

1. Copy from nets/ folder or create a parameters.h file for target CNN and drop it in the src/ folder (e.g. nets/alexnet.parameters.h --> src/paramters.h)
2. Copy from nets/ folder or create a address.h file for target CNN and drop it in the src/ folder (e.g. nets/alexnet.address.h --> src/address.h)
3. Type _make_ or ./build.sh, which will build the program
4. Use ./run.sh to run the application

Note that steps 1 and 2 rely on pre-generated versions of each implementation by hand. If you would like to create a new mapping based on an existing protobuf description of a network, support is currently under way! address.h can be generated from parameters.h using a simple C tool which can be found in the tools/ directory. The .protobuf-to-parameters.h translation is a feature currently under development and would be available shortly.
