# CaffePresso #

This git repository supports the CASES 2016 paper "Cafftimized ePresso: An Optimized Library for Deep Learning on Embedded Accelerator-based platforms". It contains ConvNet implementations for DSP, FPGA, and NoC-based embedded accelerators. The framework is modularized so support for new hardware platforms is simple. It is also possible to change the ConvNet specifications and regenerate the implementations.

### How do I get set up? ###

**Hardware** -- TI Keystone II DSP (66AK2H12), Adapteva Parallella/Epiphany-III SoC board, NVIDIA Jetson TX1 GPU, Xilinx ZC706 FPGA. Also need USB cables and SD cards as required.

### Contribution guidelines ###

* Writing tests
* Code review
* Other guidelines

### How to cite this paper? ###

If you like this tool, please cite:

```
@article{caffepresso_cases2016,
  title={CaffePresso: An Optimized Library for Deep Learning on Embedded Accelerator-based platforms},
  author={Hegde, Gopalakrishna and Siddhartha and Ramasamy, Nachiappan and Kapre, Nachiket},
 booktitle = {Proceedings of the 2016 International Conference on Compilers, Architecture and Synthesis for Embedded Systems},
  year={2016}
}
```