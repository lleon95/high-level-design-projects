SYSTEMC_HOME=/opt/systemc
GCC=g++
LD_LIBRARY_PATH=$SYSTEMC_HOME/lib-linux64/
CXXFLAGS=-Wall -Wextra
INCLUDES=-I../router/

MODULE=dac
SOURCES=$(MODULE).cpp $(MODULE)-tbb.cpp ../router/router.cpp ../router/communication.cpp analog-4bit-dac.hpp

all: $(MODULE)

$(MODULE) : $(SOURCES)
	$(GCC) $(CXXFLAGS) $(INCLUDES) $(shell pkg-config --cflags --libs systemc-ams) -o $@ $^

test:
	./$(MODULE)

clean:
	rm -f $(MODULE) $(MODULE).vcd

