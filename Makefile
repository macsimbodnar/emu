PROGNAME = emu
FLAGS = -g -O0 -Wall -Werror

ALLOBJ = log.o util.o mos6502.o opcode.o bus.o console.o

all: main build_test

mos6502.o: mos6502.cpp mos6502.hpp opcode.o
	$(CXX) $(FLAGS) -c -o mos6502.o mos6502.cpp

opcode.o: opcode.cpp mos6502.hpp
	$(CXX) $(FLAGS) -c -o opcode.o opcode.cpp

bus.o: bus.cpp bus.hpp
	$(CXX) $(FLAGS) -c -o bus.o bus.cpp

log.o: log.cpp log.hpp
	$(CXX) $(FLAGS) -c -o log.o log.cpp

util.o: util.cpp util.hpp
	$(CXX) $(FLAGS) -c -o util.o util.cpp

console.o: console.cpp console.hpp
	$(CXX) $(FLAGS) -c -o console.o console.cpp

test: build_test
	./test

build_test: $(ALLOBJ)
	$(CXX) $(FLAGS) -o test test.cpp $(ALLOBJ)

main: $(ALLOBJ)
	$(CXX) $(FLAGS) -o $(PROGNAME) main.cpp $(ALLOBJ)

clean:
	rm -rf *.o $(PROGNAME) test