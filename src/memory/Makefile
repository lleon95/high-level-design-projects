SYSTEMC_HOME=/opt/systemc
GCC=g++
LDFLAGS= -lsystemc
CXXFLAGS=-Wall -Wextra -g -o0

MODULE=memory
SOURCES=$(MODULE).cpp $(MODULE)-tbb.cpp $(MODULE).hpp


all: $(MODULE)

$(MODULE) : $(SOURCES)
		$(GCC) $(CXXFLAGS) $(INCLUDES) $(LDFLAGS) -o $@ $^

clean:
		rm -f $(MODULE)
