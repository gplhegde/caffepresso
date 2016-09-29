#!/bin/zsh

ESDK=${EPIPHANY_HOME}
ELIBS=${ESDK}/tools/host/lib
EINCS=${ESDK}/tools/host/include
ELDF=${ESDK}/bsps/current/fast.ldf

mkdir -p bin

# Build HOST side application
gcc -O3 host.c -o bin/host.elf -I ${EINCS} -L ${ELIBS} -le-hal -lpapi -le-loader

# Build DEVICE side program
e-gcc -T ${ELDF} -O3 -mfp-mode=round-nearest pe.c -o bin/pe.elf -le-lib 
 
# Convert ebinary to SREC file
e-objcopy --srec-forceS3 --output-target srec bin/pe.elf bin/pe.srec
