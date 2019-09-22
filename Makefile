PROGNAME = emu
FLAGS = -g -O0 -Wall -Werror

ALLOBJ = log.o util.o mos6502.o bus.o 

all: main

mos6502.o: mos6502.cpp mos6502.hpp
	$(CXX) $(FLAGS) -c -o mos6502.o mos6502.cpp

bus.o: bus.cpp bus.hpp
	$(CXX) $(FLAGS) -c -o bus.o bus.cpp

log.o: log.cpp log.hpp
	$(CXX) $(FLAGS) -c -o log.o log.cpp

util.o: util.cpp util.hpp
	$(CXX) $(FLAGS) -c -o util.o util.cpp

main: $(ALLOBJ)
	$(CXX) $(FLAGS) -o $(PROGNAME) main.cpp $(ALLOBJ)

clean:
	rm -rf *.o $(PROGNAME)