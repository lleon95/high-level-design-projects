SYSTEMC_HOME=/opt/systemc
GCC=g++
LDFLAGS= -lsystemc
CXXFLAGS=-Wall -Wextra

MODULE=vga-encoder
SOURCES=$(MODULE).cpp $(MODULE)-tbb.cpp $(MODULE).hpp


all: $(MODULE)

$(MODULE) : $(SOURCES)
	$(GCC) $(CXXFLAGS) $(INCLUDES) $(LDFLAGS) $(shell pkg-config --cflags --libs systemc) -o $@ $^

clean:
	rm -f $(MODULE)
