PROGNAME = emu
FLAGS = -g -O0 -Wall -Werror

ALLOBJ = log.o util.o mos6502.o opcode.o bus.o console.o cartridge.o mapper_000.o

all: main

mos6502.o: mos6502.cpp mos6502.hpp common.hpp opcode.o
	$(CXX) $(FLAGS) -c -o mos6502.o mos6502.cpp

opcode.o: opcode.cpp mos6502.hpp
	$(CXX) $(FLAGS) -c -o opcode.o opcode.cpp

bus.o: bus.cpp bus.hpp
	$(CXX) $(FLAGS) -c -o bus.o bus.cpp

cartridge.o: cartridge.cpp cartridge.hpp
	$(CXX) $(FLAGS) -c -o cartridge.o cartridge.cpp

mapper_000.o: mapper_000.cpp mapper_000.hpp mapper.hpp
	$(CXX) $(FLAGS) -c -o mapper_000.o mapper_000.cpp

log.o: log.cpp log.hpp
	$(CXX) $(FLAGS) -c -o log.o log.cpp

util.o: util.cpp util.hpp
	$(CXX) $(FLAGS) -c -o util.o util.cpp

console.o: console.cpp console.hpp
	$(CXX) $(FLAGS) -c -o console.o console.cpp

main: $(ALLOBJ)
	$(CXX) $(FLAGS) -o $(PROGNAME) main.cpp $(ALLOBJ)

build_test: test.cpp $(ALLOBJ)
	$(CXX) $(FLAGS) -o test test.cpp $(ALLOBJ)

test: build_test
	./test

clean:
	rm -rf *.o $(PROGNAME) test