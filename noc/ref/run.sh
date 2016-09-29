#!/bin/zsh

ESDK=${EPIPHANY_HOME}
ELIBS=${ESDK}/tools/host/lib:${LD_LIBRARY_PATH}
EHDF=${EPIPHANY_HDF}

cd bin
LD_LIBRARY_PATH=${ELIBS}; ./host.elf $1 $2 $3 $4

