SYSTEMC_HOME=/opt/systemc
GCC=g++
LDFLAGS= -lsystemc
CXXFLAGS=-Wall -Wextra
DEFINES=-DDEBUG

MODULE=vga_decoder
SOURCES=$(MODULE).cpp $(MODULE)_tbb.cpp ../router/router.cpp ../router/communication.cpp
INCLUDES=-I../router/


all: $(MODULE)

$(MODULE) : $(SOURCES)
	$(GCC) $(CXXFLAGS) $(INCLUDES) $(DEFINES) $(LDFLAGS) -o $@ $^

test:
	./$(MODULE)

clean:
	rm -f $(MODULE)
