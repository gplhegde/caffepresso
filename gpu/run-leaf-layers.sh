#!/bin/bash
gpu_no_iter=1
cpu_no_iter=1
LOG="leaf_layer_runtime_log.txt.`date +'%Y-%m-%d_%H-%M-%S'`"
echo Logging output to "$LOG"
exec &> >(tee -a "$LOG")

echo "Running each benchmark on GPU $gpu_no_iter times"
echo "Running each benchmark on CPU $cpu_no_iter times"
echo "--------Starting Caffe Deep Learning Benchmark runs-----------"
echo "---------GPU RUNS---------"
echo "===================================================================================="
echo "---------CONV LAYER START-----"
caffe time --model=./conv.prototxt --gpu=0 -iterations=$gpu_no_iter
echo "---------CONV END-------"
echo "===================================================================================="
echo "---------POOL LAYER START-----"
caffe time --model=./pool.prototxt --gpu=0 -iterations=$gpu_no_iter
echo "---------POOL LAYER END-------"
echo "===================================================================================="
echo "---------ReLU LAYER START-----"
caffe time --model=./relu.prototxt --gpu=0 -iterations=$gpu_no_iter
echo "---------ReLU LAYER  END-------"
echo "===================================================================================="
echo "---------FC LAYER START-----"
caffe time --model=./fc_layer.prototxt --gpu=0 -iterations=$gpu_no_iter
echo "---------FC LAYER END-------"
echo "===================================================================================="
echo "---------CPU RUNS---------"
echo "===================================================================================="
echo "---------CONV LAYER START-----"
caffe time --model=./conv.prototxt -iterations=$cpu_no_iter
echo "---------CONV END-------"
echo "===================================================================================="
echo "---------POOL LAYER START-----"
caffe time --model=./pool.prototxt  -iterations=$cpu_no_iter
echo "---------POOL LAYER END-------"
echo "===================================================================================="
echo "---------ReLU LAYER START-----"
caffe time --model=./relu.prototxt  -iterations=$cpu_no_iter
echo "---------ReLU LAYER  END-------"
echo "===================================================================================="
echo "---------FC LAYER START-----"
caffe time --model=./fc_layer.prototxt  -iterations=$cpu_no_iter
echo "---------FC LAYER END-------"
echo "===================================================================================="
echo "Ran each benchmark on GPU $gpu_no_iter times"
echo "Ran each benchmark on CPU $cpu_no_iter times"
echo "---------THATS ALL-----------"
echo "Logs are stored to $LOG"

echo "Final results-->"
echo $LOG | grep "forward:" | grep -v "input" | sed "s/.*]//g"
