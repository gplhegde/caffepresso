#Running all benchmarks on GPU platforms : Instructions

1. Make sure that Caffe is compiled with cuDNN=1 flag
2. To run tests execute the following command
**./run-cases-benchmarks.sh**  *caffe_installation_path*

3. Logs will be stored in this directory.
4. No of iterations on CPU and GPU can be changed by modifying cpu_no_iter and gpu_no_iter in the script

