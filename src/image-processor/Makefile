SYSTEMC_HOME=/usr/local/systemc
GCC=g++
LDFLAGS= -lsystemc  -L$(SYSTEMC_HOME)/lib-linux64
CXXFLAGS=-Wall -Wextra
INCLUDES=-I../router/ -I$(SYSTEMC_HOME)/include

MODULE=image-processor
SOURCES=$(MODULE).cpp $(MODULE)-tbb.cpp ../router/router.cpp


all: $(MODULE)

$(MODULE) : $(SOURCES)
	$(GCC) $(CXXFLAGS) $(INCLUDES) $(LDFLAGS) -o $@ $^

test:
	./$(MODULE)

clean:
	rm -f $(MODULE) $(MODULE).vcd
