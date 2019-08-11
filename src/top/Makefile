PROJECT=top

SYSTEMC_HOME=/opt/systemc
GCC=g++
CXXFLAGS=-Wall -Wextra
DEFINES=-DDEBUG

SOURCES=../image-processor/image-processor.cpp  ../dac/dac.cpp ../dac/analog-4bit-dac.hpp  ../router/router.cpp ../vga-decoder/vga_decoder.cpp ../vga-encoder/vga-encoder.cpp ../router/router.cpp top-tbb.cpp ../adc/adc.cpp ../vga-decoder/vga_decoder.cpp ../router/communication.cpp
INCLUDES=-I../image-processor/  -I../dac/ -I../router/ -I../vga-decoder/ -I../vga-encoder/ -I../router/ -I../vga-decoder/ -I../adc/ -I$(SYSTEMC_HOME)/include/

all: $(PROJECT)

$(PROJECT) : $(SOURCES)
	$(GCC) $(CXXFLAGS) $(DEFINES) $(INCLUDES) $^ $(shell pkg-config --cflags --libs systemc-ams) -o $@

test:
	./$(MODULE)

clean:
	rm -f $(PROJECT)
