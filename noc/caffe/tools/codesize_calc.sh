#!/usr/bin/zsh

# idea from: http://parallella.org/forums/viewtopic.php?f=13&t=702
# supply path to srec file to be analyzed
out=`e-objdump -h $1`

count=`echo $out | grep -B1 "ALLOC" | wc -l`
num=`echo "${count}/2" | bc`

size=0

for ((i=0;i<${num};i++))
do
	index=`echo "${i}*2+1" | bc`
	c=`echo $out | grep -B1 "ALLOC" | head -n ${index} | tail -n 1 | sed "s/ \{1,\}/ /g" | cut -d " " -f 6`
	c=`echo ${c} | awk '{print toupper($0)}'`
	if [[ `echo "ibase=16;obase=16;8000-${c}" | bc | grep "-" | wc -l` -eq 0 ]]
	then
		s=`echo $out | grep -B1 "ALLOC" | head -n ${index} | tail -n 1 | sed "s/ \{1,\}/ /g" | cut -d " " -f 4`
		s=`echo ${s} | awk '{print toupper($0)}'`
		echo "in size = ${s}"
		size=`echo "obase=16;ibase=16;${size}+${s}" | bc`
		echo "total size = 0x${size}"
	fi
done
echo "FINAL=0x${size}"
