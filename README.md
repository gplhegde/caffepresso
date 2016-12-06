# CaffePresso #

This git repository supports the CASES 2016 paper **"CaffePresso: An Optimized Library for Deep Learning on Embedded Accelerator-based platforms"**. It contains ConvNet implementations for DSP, FPGA, and NoC-based embedded accelerators. We provide a code generator that translates Caffe prototxt into low-level specifications for the various backends.

The framework is modularized so support for new hardware platforms is simple. It is also possible to change the ConvNet specifications and regenerate the implementations.

### What are the pre-requisites to use this repo? ###

**Hardware** (USD prices from May 2016)

- TI Keystone II DSP (66AK2H12) -- $997
- Adapteva Parallella/Epiphany-III SoC board -- $126
- NVIDIA Jetson TX1 GPU platform -- $599
- Xilinx ZC706 FPGA -- $2275
- Also need USB cables (TI XDS100 programming cable) and SD cards as required.

**Software**

- TI Code Composer Studio v6, IMGLIB and DSPLIB
- Epiphany SDK (https://github.com/adapteva/epiphany-sdk)
- CUDA + cuDNNv4 libraries
- Vectorblox bitstreams + Xilinx Vivado (https://github.com/VectorBlox/mxp/tree/master/examples/boards/zc706_arm_viv)
- OS images for Parallella (https://www.parallella.org/create-sdcard/)
- Caffe from caffe.berkeleyvision.org. Make sure to build it with CUDNN:=1 flag.

### Building and running code ###

The ConvNet configurations used in the paper are stored in **nets** folder. The code-generation scripts are in **tools/caffe-proto** folders. 

**gpu** -- This contains the scripts to run various ConvNets via Caffe + cuDNN on the Jetson TX1. It is a seamless experience and it will get your started right away.

**mxp** -- You can either do a simulation (via license from Vectorblox) or directly execute on the FPGA board with the dowloaded 64-lane ZC706 bitstream (freely available on Vectorblox github). The header and top-level files are assembled from the code generator block.

**dsp** and **noc** -- These contain sub-folders for the various datasets. There is some manual assembly required to run this through the respective build systems. The Caffe code-generator frontend supplies the ConvNet-specific header configuration for execution.

### How to cite this paper? ###

If you use this tool in your work and find it useful, please cite:

```
@article{caffepresso_cases2016,
  title={CaffePresso: An Optimized Library for Deep Learning on Embedded Accelerator-based platforms},
  author={Hegde, Gopalakrishna and Siddhartha and Ramasamy, Nachiappan and Kapre, Nachiket},
  booktitle = {Proceedings of the 2016 International Conference on Compilers, Architecture and Synthesis for Embedded Systems},
  year={2016}
}
```
