#!/bin/sh
echo 'Compiling *.c *cpp files'
rm -rf voltage.o
export SYSTEMC_HOME=/opt/systemc/
export LD_LIBRARY_PATH=$SYSTEMC_HOME/lib-linux64/
g++ -I$SYSTEMC_HOME/include -L$SYSTEMC_HOME/lib-linux64/ analog-4bit-dac-tbb.cpp -lsystemc -lsystemc-ams -lm -o voltage.o -O0 -g
echo 'Simulation Started'
./voltage.o
echo 'Simulation Ended'
