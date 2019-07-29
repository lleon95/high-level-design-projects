SYSTEMC_HOME=/opt/systemc
GCC=g++
LDFLAGS= -lsystemc
CXXFLAGS=-Wall -Wextra
INCLUDES=-I../router/

MODULE=dac
SOURCES=$(MODULE).cpp $(MODULE)-tbb.cpp ../router/router.cpp

all: $(MODULE)

$(MODULE) : $(SOURCES)
	$(GCC) $(CXXFLAGS) $(INCLUDES) $(LDFLAGS) $(shell pkg-config --cflags --libs systemc) -o $@ $^

test:
	./$(MODULE)

clean:
	rm -f $(MODULE) $(MODULE).vcd

