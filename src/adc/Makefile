SYSTEMC_HOME=/opt/systemc
GCC=g++
LD_LIBRARY_PATH=$SYSTEMC_HOME/lib-linux64/
CXXFLAGS=-Wall -Wextra
INCLUDES=-I../vga-decoder/ -I../router/

MODULE=adc
SOURCES=$(MODULE).cpp $(MODULE)-tbb.cpp adc_4_bits.cpp ../router/router.cpp ../router/communication.cpp

all: $(MODULE)

$(MODULE) : $(SOURCES)
	$(GCC) $(CXXFLAGS) $(INCLUDES)  $^ $(shell pkg-config --cflags --libs systemc-ams) -o $@

test:
	./$(MODULE)

clean:
	rm -f $(MODULE) $(MODULE).vcd

