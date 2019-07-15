SYSTEMC_HOME=/opt/systemc
GCC=g++
LDFLAGS= -lsystemc -L $(SYSTEMC_HOME)/lib-linux64
INCLUDES= -I$(SYSTEMC_HOME)/include
CXXFLAGS=-Wall -Wextra -g -o0

MODULE=memory
SOURCES=$(MODULE).cpp $(MODULE)-tbb.cpp $(MODULE).hpp


all: $(MODULE)

$(MODULE) : $(SOURCES)
		$(GCC) $(CXXFLAGS) $(INCLUDES) $(LDFLAGS) -o $@ $^

clean:
		rm -f $(MODULE)
