#!/bin/zsh

gpu_no_iter=10
cpu_no_iter=10
LOG="cases_jetson_benchmark_logs.txt.`date +'%Y-%m-%d_%H-%M-%S'`"

for bench in mnist cifar10 caltech101 stl10 alexnet 
do
	caffe time --model=./${bench}.prototxt --gpu=0 -iterations=$gpu_no_iter &> ${bench}.gpu.$LOG
	caffe time --model=./${bench}.prototxt -iterations=$cpu_no_iter       	&> ${bench}.cpu.$LOG
done

for bench in mnist cifar10 caltech101 stl10 alexnet 
do
	cat ${bench}.gpu.$LOG | grep "Average Forward pass" | sed "s/.*]/$bench,gpu,/g" | sed "s/Average Forward pass://"
	cat ${bench}.cpu.$LOG | grep "Average Forward pass" | sed "s/.*]/$bench,cpu,/g" | sed "s/Average Forward pass://"
done
