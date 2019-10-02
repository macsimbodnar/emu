PROGNAME = emu
FLAGS = -g -O0 -Wall -Werror -std=c++17

DOCTEST = -I ./libs

ALLOBJ = mos6502.o opcode.o util.o

all: console.o build_test

mos6502.o: mos6502.cpp mos6502.hpp common.hpp opcode.o
	$(CXX) $(FLAGS) -c -o mos6502.o mos6502.cpp

opcode.o: opcode.cpp mos6502.hpp
	$(CXX) $(FLAGS) -c -o opcode.o opcode.cpp

util.o: util.cpp util.hpp
	$(CXX) $(FLAGS) -c -o util.o util.cpp

console.o: console.cpp console.hpp $(ALLOBJ)
	$(CXX) $(FLAGS) -o $(PROGNAME) console.cpp $(ALLOBJ)

build_test: test.cpp $(ALLOBJ)
	$(CXX) $(FLAGS) $(DOCTEST) -o test test.cpp $(ALLOBJ)

test: build_test
	./test

clean:
	rm -rf *.o $(PROGNAME) test