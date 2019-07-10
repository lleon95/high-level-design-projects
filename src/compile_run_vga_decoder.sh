#!/bin/sh
echo 'Compiling *.c *cpp files'
rm -rf vga_decoder.o
export SYSTEMC_HOME=/usr/local/systemc-2.3.3/
export LD_LIBRARY_PATH=$SYSTEMC_HOME/lib-linux64
g++ -I$SYSTEMC_HOME/include -L$SYSTEMC_HOME/lib-linux64 vga_decoder.cpp  -lsystemc -lm -o vga_decoder.o
echo 'Simulation Started'
./memory.o
echo 'Simulation Ended'
