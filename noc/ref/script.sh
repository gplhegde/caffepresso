#!/bin/zsh

rm -f script.csv
touch -f script.csv

echo "Function, NUM_ROW, NUM_COL, NUM_KERNEL_ROW, NUM_KERNEL_COL, time" > script.csv
#for i in filter2D subsample interpolate add sub normalize
for i in filter2D
do
	cp pe_$i.c pe.c
	./build.sh
	# sizes above 32 currently fail due to memory layout errors..
	k=3
	for j in 4 8 16 32
	do 
		./run.sh $j $j $k $k > /tmp/wtf.txt
		time=`cat /tmp/wtf.txt | grep "PAPI" | sed "s/.*Time=\(.*\)us/\1/"`
		cycles=`cat /tmp/wtf.txt | grep "Clocks" | sed "s/.*clocks=\(.*\) cycles/\1/"`
		echo "$i,$j,$j,$k,$k,$time,$cycles" >> script.csv
	done
done
