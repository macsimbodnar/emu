PROGNAME = emu
OUTPUT_DIR = .
FLAGS = -g -O0 -Wall -Werror -std=c++17

DOCTEST = -I ./libs

ALLOBJ = mos6502.o opcode.o util.o

all: console build_test

%.o: %.cpp
	$(CXX) $(FLAGS) -c -o $@ $<

console: console.cpp console.hpp $(ALLOBJ)
	$(CXX) $(FLAGS) -o $(PROGNAME) console.cpp $(ALLOBJ)

build_test: test.cpp $(ALLOBJ)
	$(CXX) $(FLAGS) $(DOCTEST) -o test test.cpp $(ALLOBJ)

test: build_test 
	./test

clean:
	rm -rf *.o $(PROGNAME) test