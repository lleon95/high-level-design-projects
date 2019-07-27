SYSTEMC_HOME=/opt/systemc
GCC=g++
LDFLAGS= -lsystemc
CXXFLAGS=-Wall -Wextra

MODULE=image_processor
SOURCES=$(MODULE).cpp $(MODULE)-tbb.cpp


all: $(MODULE)

$(MODULE) : $(SOURCES)
	$(GCC) $(CXXFLAGS) $(INCLUDES) $(LDFLAGS) -o $@ $^

test:
	./$(MODULE)

clean:
	rm -f $(MODULE) $(MODULE).vcd
