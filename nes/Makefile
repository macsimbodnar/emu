PROGNAME = nes
FLAGS = -g -O0 -Wall -Werror -std=c++17 -I../

MOS6502_DIR = ..
ALLOBJ = $(patsubst %.o,$(MOS6502_DIR)/%.o,    mos6502.o opcode.o util.o)
ALLOBJ += cartridge.o ppu.o cpu.o nes.o

all: main

# Build for the current directory
%.o: %.cpp
	$(CXX) $(FLAGS) -c -o $@ $<

# Build for the MOS6502_DIR directory
$(MOS6502_DIR)/%:
	$(MAKE) $(@F) -C $(@D)

# Build the main
main: $(ALLOBJ)
	$(CXX) $(FLAGS) -o $(PROGNAME) main.cpp $(ALLOBJ)

clean:
	rm -rf *.o $(PROGNAME)