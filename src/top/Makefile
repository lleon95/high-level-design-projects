PROJECT=top

SYSTEMC_HOME=/opt/systemc
GCC=g++
LDFLAGS= -lsystemc
CXXFLAGS=-Wall -Wextra
DEFINES=-DEBUG

SOURCES=../image-processor/image-processor.cpp  ../dac/dac.cpp  ../memory/memory.cpp ../router/router.cpp ../vga-decoder/vga_decoder.cpp ../vga-encoder/vga-encoder.cpp ../router/router.cpp top-tbb.cpp ../adc/adc.cpp ../vga-decoder/vga_decoder.cpp
INCLUDES=-I../image-processor/  -I../dac/  -I../memory/ -I../router/ -I../vga-decoder/ -I../vga-encoder/ -I../router/ -I../vga-decoder/ -I../adc/

all: $(PROJECT)

$(PROJECT) : $(SOURCES)
	$(GCC) $(CXXFLAGS) $(DEFINES) $(INCLUDES) $(LDFLAGS) -o $@ $^

test:
	./$(MODULE)

clean:
	rm -f $(PROJECT)
