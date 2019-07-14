SYSTEMC_HOME=/opt/systemc
GCC=g++
LDFLAGS= -lsystemc
CXXFLAGS=-Wall -Wextra

MODULE=vga_decoder
SOURCES=$(MODULE).cpp $(MODULE)_tbb.cpp

all: $(MODULE)

$(MODULE) : $(SOURCES)
	$(GCC) $(CXXFLAGS) $(INCLUDES) $(LDFLAGS) $(shell pkg-config --cflags --libs systemc) -o $@ $^

test:
	./$(MODULE)

clean:
	rm -f $(MODULE)
