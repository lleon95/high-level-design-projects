PROJECT=top

SYSTEMC_HOME=/usr/local/systemc
GCC=g++
LDFLAGS= -lsystemc -L$(SYSTEMC_HOME)/lib-linux64
CXXFLAGS=-Wall -Wextra
DEFINES=-DDEBUG

SOURCES=../image-processor/image-processor.cpp ../memory/memory.cpp ../dac/dac.cpp  ../router/router.cpp ../vga-decoder/vga_decoder.cpp ../vga-encoder/vga-encoder.cpp ../router/router.cpp top-tbb.cpp ../adc/adc.cpp ../vga-decoder/vga_decoder.cpp
INCLUDES=-I../image-processor/  -I../dac/ -I../router/ -I../vga-decoder/ -I../vga-encoder/ -I../router/ -I../vga-decoder/ -I../adc/ -I$(SYSTEMC_HOME)/include/ -I../memory/

all: $(PROJECT)

$(PROJECT) : $(SOURCES)
	$(GCC) $(CXXFLAGS) $(DEFINES) $(INCLUDES) $(LDFLAGS) -o $@ $^

test:
	./$(MODULE)

clean:
	rm -f $(PROJECT)
