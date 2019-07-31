SYSTEMC_HOME=/opt/systemc
GCC=g++
LDFLAGS= -lsystemc
CXXFLAGS=-Wall -Wextra
INCLUDES=-I../router/

MODULE=vga-encoder
SOURCES=$(MODULE).cpp $(MODULE)-tbb.cpp ../router/router.cpp ../router/communication.cpp

all: $(MODULE)

$(MODULE) : $(SOURCES)
	$(GCC) $(CXXFLAGS) $(INCLUDES) $(LDFLAGS) $(shell pkg-config --cflags --libs systemc) -o $@ $^

test:
	./$(MODULE)

clean:
	rm -f $(MODULE)

