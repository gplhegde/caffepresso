# Jetson TX1 Instructions

1. We assume the CUDA and cuDNN libraries are installed. You will also need to install Caffe. Make sure that it is compiled with cuDNN=1 flag

2. To run tests execute the following command
```./run-cases-benchmarks.sh <caffe_installation_path>```

3. The script will loop through the different prototxt specifications and generate timing results. These result logs will be stored in this directory itself.

4. The number of iterations on CPU and GPU can be changed by modifying cpu_no_iter and gpu_no_iter in the script. This is important to average your runtime scores.

