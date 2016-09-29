#!/usr/bin/zsh

touch sweep_parallella.csv
filter2D_unroll=0

for opt in LOW_PRECISION
do
	#cached values to avoid unnecessary recompiles
	cimg=0
	ckernel=0
	cdown=0
	cunroll=0
	cmap_width=0
	
	while read line
	do
		if [[ `echo ${line} | grep "#" | wc -l` -ne 1 ]]
		then
			img=`echo ${line} | cut -d"," -f 1`
			kernel=`echo ${line} | cut -d"," -f 2`
			down=`echo ${line} | cut -d"," -f 3`
			unroll=`echo ${line} | cut -d"," -f 4`
			maps=`echo ${line} | cut -d"," -f 5`
			addr1=`echo ${line} | cut -d"," -f 6`
			addr2=`echo ${line} | cut -d"," -f 7`
			addr3=`echo ${line} | cut -d"," -f 8`
			addr4=`echo ${line} | cut -d"," -f 9`
			addr5=`echo ${line} | cut -d"," -f 10`
			addr6=`echo ${line} | cut -d"," -f 11`
			addr7=`echo ${line} | cut -d"," -f 12`
			free=`echo ${line} | cut -d"," -f 13`
			map_width=`echo ${line} | cut -d"," -f 14`
	
			if [[ ${cimg} -ne ${img} ||  ${ckernel} -ne ${kernel} || ${cdown} -ne ${down} || ${cunroll} -ne ${unroll} || ${cmap_width} -ne ${map_width} ]]
			then
				echo "[${opt}] Running [${img}x${img}] patch, [${kernel}x${kernel}] kernel, [${map_width}x${map_width}] maps, down sampling factor = ${down}, maps unroll = ${unroll}"
				echo "\tCreating parameters header file..."
				#create parameters file
				rm parameters.h
				touch parameters.h
				echo "#define KERNEL_WIDTH ${kernel}" >> parameters.h 
				echo "#define KERNEL_SIZE (${kernel}*${kernel})" >> parameters.h
				echo "#define MAP_SIZE (${map_width}*${map_width})" >> parameters.h
				echo "#define IMAGE_SIZE (${img}*${img})" >> parameters.h
				echo "#define IMAGE_WIDTH ${img}" >> parameters.h
				echo "#define MAP_UNROLL_FACTOR ${unroll}" >> parameters.h
				echo "#define DOWN_FAC ${down}" >> parameters.h

				echo "\tCreating addresses header file..."
				#create address.h file
				rm address.h
				touch address.h
				echo "#define GLOBAL_CONSTANTS_ADDR ${addr1}" >> address.h
				echo "#define IMAGE_ADDR ${addr2}" >> address.h
				echo "#define KERNEL_ADDR ${addr3}" >> address.h
				echo "#define KERNEL_SCALE_ADDR ${addr4}" >> address.h
				echo "#define MAPS_ADDR ${addr5}" >> address.h
				echo "#define DONE_ADDR ${addr6}" >> address.h
				echo "#define SCRATCHPAD_ADDR ${addr7}" >> address.h

				echo "\tCopying over implementation files..."
				#copy over relevant files
				if [[ ${filter2D_unroll} -eq 1 ]]
				then
					cp ${opt}/pe_unroll_${kernel}.c ./pe.c
				else
					cp ${opt}/pe_nounroll.c ./pe.c
				fi
				cp ${opt}/macros.h .
				cp ${opt}/types.h .
			
				echo "\tCompiling..."
				#compile
				./build.sh
			
				#safety check
				codesize=`./tools/codesize.sh bin/pe.srec | grep "FINAL" | cut -d"=" -f 2`
				echo "\tCodesize = ${codesize}, Starting address = ${addr1}..."
				codesizer=$((${codesize}+0x100)) #at least 256B more than original
				if [[ ${codesizer} -ge ${addr1} ]]
				then
					if [[ ${codesize} -ge ${addr1} ]]
					then
						diff=`printf "0x%x\n" $((${codesize}-${addr1}))`
					else
						diff=0
					fi
					diff=`printf "0x%x\n" $((${diff}+0x100))` #add 256B buffer
					#just for sanity, must be byte-aligned
					mod=$(((${diff}+${addr1})%4))
					diff=`printf "0x%x\n" $((${diff}+${mod}))` #byte-alignment
					if [[ ${free} -le ${diff} ]]
					then
						continue
					else
						echo "\tRecalibrating addresses by ${diff} bytes..."
						addr1=`printf "0x%x\n" $((${addr1}+${diff}))`
						addr2=`printf "0x%x\n" $((${addr2}+${diff}))`
						addr3=`printf "0x%x\n" $((${addr3}+${diff}))`
						addr4=`printf "0x%x\n" $((${addr4}+${diff}))`
						addr5=`printf "0x%x\n" $((${addr5}+${diff}))`
						addr6=`printf "0x%x\n" $((${addr6}+${diff}))`
						addr7=`printf "0x%x\n" $((${addr7}+${diff}))`
						
						#re-create address.h file
						rm address.h
						touch address.h
						echo "#define GLOBAL_CONSTANTS_ADDR ${addr1}" >> address.h
						echo "#define IMAGE_ADDR ${addr2}" >> address.h
						echo "#define KERNEL_ADDR ${addr3}" >> address.h
						echo "#define KERNEL_SCALE_ADDR ${addr4}" >> address.h
						echo "#define MAPS_ADDR ${addr5}" >> address.h
						echo "#define DONE_ADDR ${addr6}" >> address.h
						echo "#define SCRATCHPAD_ADDR ${addr7}" >> address.h

						echo "\tRe-compiling..."
						./build.sh
					fi
				fi

				#update cached values
				cimg=${img}
				ckernel=${kernel}
				cdown=${down}
				cunroll=${unroll}
				cmap_width=${map_width}
			fi

			echo "\tRunning maps = ${maps}..."
			runtime=`./run.sh ${maps} | grep "Runtime" | cut -d"=" -f 2`

			echo "${opt},${img},${kernel},${maps},${map_width},${down},${unroll},${runtime}" >> sweep_parallella.csv
		fi
	done < why.csv
done
