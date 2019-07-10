SYSTEMC_HOME=/opt/systemc
GCC=g++
LDFLAGS= -lsystemc
CXXFLAGS=-Wall -Wextra

MODULE=image-processor
SOURCES=$(MODULE).cpp $(MODULE)-tbb.cpp


all: $(MODULE)

$(MODULE) : $(SOURCES)
	$(GCC) $(CXXFLAGS) $(INCLUDES) $(LDFLAGS) -o $@ $^

clean:
	rm -f $(MODULE)
