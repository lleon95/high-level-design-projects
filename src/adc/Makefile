SYSTEMC_HOME=/opt/systemc
GCC=g++
LDFLAGS= -lsystemc
CXXFLAGS=-Wall -Wextra
DEFINES=-DDEBUG

MODULE=adc
SOURCES=$(MODULE).cpp $(MODULE)_tbb.cpp ../router/router.cpp
INCLUDES=-I../router/


all: $(MODULE)

$(MODULE) : $(SOURCES)
	$(GCC) $(CXXFLAGS) $(INCLUDES) $(DEFINES) $(LDFLAGS) -o $@ $^

test:
	./$(MODULE)

clean:
	rm -f $(MODULE)
