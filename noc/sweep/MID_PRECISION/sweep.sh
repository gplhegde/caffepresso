#!/usr/bin/zsh

### INPUT
#int IMG_WIDTH = atoi(argv[1]);
#int maps_per_ecore = atoi(argv[2]);
#int KERNEL_WIDTH = atoi(argv[3]);
#int DOWN_FAC = atoi(argv[4]);
#int MAP_UNROLL_FACTOR = atoi(argv[5]);


### OUTPUT
#FREE SPACE=10416

#define GLOBAL_CONSTANTS_ADDR 0x4000
#define IMG_ADDR 0x4004
#define KERNEL_ADDR 0x4314
#define KERNEL_SCALE_ADDR 0x43d8
#define MAPS_ADDR 0x43e8
#define DONE_ADDR 0x47b0
#define SCRATCHPAD_ADDR 0x47c0

for opt in MID_PRECISION 
do
	gcc -D$opt ../tools/address.c -omem
	
	rm ${opt}_FILTER2D_UNROLL.csv
	touch ${opt}_FILTER2D_UNROLL.csv
	echo "#img_width,kernel_width,down_fac,map_unroll,maps,addr1,addr2,addr3,addr4,addr5,addr6,addr7,free,map_width" >> ${opt}_FILTER2D_UNROLL.csv

	for ((img=4;img<65;img+=2))
	do
		for kernel in 3 5 7 9 11
		do
			START_ADDR=`cat sizes.csv | grep "${opt},${kernel}" | cut -d"," -f 3`
			for down_fac in 2 # 3 4 5
			do
				for map_unroll in 1 # 2 3 4 5
				do
					success=0
					maps=1
					while [[ $success -ne 1 ]]
					do
						echo -n "[${opt}] Running IMG=${img},KERNEL=${kernel},DOWN_FAC=${down_fac},MAP_UNROLL=${map_unroll},MAPS=${maps}.."
						out=`./mem ${img} ${maps} ${kernel} ${down_fac} ${map_unroll} ${START_ADDR}`
						suc=`echo ${out} | grep "SUCCESS" | wc -l`
						if [[ ${suc} -eq 1 ]]
						then
							free=`echo ${out} | grep "FREE SPACE" | cut -d"=" -f2`
							addr1=`echo ${out} | grep "GLOBAL_CONSTANTS_ADDR" | cut -d" " -f3`
							addr2=`echo ${out} | grep "IMG_ADDR" | cut -d" " -f3`
							addr3=`echo ${out} | grep "KERNEL_ADDR" | cut -d" " -f3`
							addr4=`echo ${out} | grep "KERNEL_SCALE_ADDR" | cut -d" " -f3`
							addr5=`echo ${out} | grep "MAPS_ADDR" | cut -d" " -f3`
							addr6=`echo ${out} | grep "DONE_ADDR" | cut -d" " -f3`
							addr7=`echo ${out} | grep "SCRATCHPAD_ADDR" | cut -d" " -f3`
							map_width=`echo ${out} | grep "MAP WIDTH" | cut -d"=" -f2`
							echo "${img},${kernel},${down_fac},${map_unroll},${maps},${addr1},${addr2},${addr3},${addr4},${addr5},${addr6},${addr7},${free},${map_width}" >> ${opt}_FILTER2D_UNROLL.csv
							echo "SUCCESS."
							maps=$((${maps}+1))
						else
							echo "FAIL."
							success=1
						fi
					done
				done
			done
		done
	done
done

rm mem
