PROGNAME = emu
FLAGS = -g -O0

ALLOBJ = mos6502.o bus.o

all: main

mos6502.o: mos6502.cpp mos6502.hpp
	$(CXX) $(FLAGS) -c -o mos6502.o mos6502.cpp

bus.o: bus.cpp bus.hpp
	$(CXX) $(FLAGS) -c -o bus.o bus.cpp

main: bus.o mos6502.o
	$(CXX) $(FLAGS) -o $(PROGNAME) main.cpp $(ALLOBJ)

clean:
	rm *.o $(PROGNAME)