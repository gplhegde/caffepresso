#!/bin/bash

set -e

ESDK=${EPIPHANY_HOME}
ELIBS=${ESDK}/tools/host/lib
EINCS=${ESDK}/tools/host/include
ELDF=${ESDK}/bsps/current/fast.ldf

# Create the binaries directory
mkdir -p bin/

CROSS_PREFIX=
case $(uname -p) in
	arm*)
		# Use native arm compiler (no cross prefix)
		CROSS_PREFIX=
		;;
	   *)
		# Use cross compiler
		CROSS_PREFIX="arm-linux-gnueabihf-"
		;;
esac

# Build HOST side application
${CROSS_PREFIX}gcc -I/usr/local/include -O3 host.c /usr/local/lib/libpapi.a -o bin/host.elf -I ${EINCS} -L ${ELIBS} -lm -lrt -le-hal #-le-loader

# Build DEVICE side program
e-gcc -finline-functions -ffast-math -T ${ELDF} -O3 pe_layer.c -o bin/pe.elf -le-lib

# Convert ebinary to SREC file
e-objcopy --srec-forceS3 --output-target srec bin/pe.elf bin/pe.srec
