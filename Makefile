PROGNAME = emu
OUTPUT_DIR = ./build
FLAGS = -g -O0 -Wall -Werror -std=c++17

SDL = `pkg-config --cflags --libs sdl2`

DOCTEST = -I ./libs

ALLOBJ = $(OUTPUT_DIR)/mos6502.o $(OUTPUT_DIR)/opcode.o $(OUTPUT_DIR)/util.o $(OUTPUT_DIR)/pixello.o

all: console.o build_test build_test_pixello

all_objects: mos6502.o opcode.o util.o pixello.o

mos6502.o: mos6502.cpp mos6502.hpp common.hpp opcode.o util.o
	$(CXX) $(FLAGS) -c -o $(OUTPUT_DIR)/mos6502.o mos6502.cpp

opcode.o: opcode.cpp mos6502.hpp
	$(CXX) $(FLAGS) -c -o $(OUTPUT_DIR)/opcode.o opcode.cpp

util.o: util.cpp util.hpp
	$(CXX) $(FLAGS) -c -o $(OUTPUT_DIR)/util.o util.cpp

pixello.o: pixello.cpp pixello.hpp
	$(CXX) $(FLAGS) -c -o $(OUTPUT_DIR)/pixello.o pixello.cpp $(SDL)

console.o: console.cpp console.hpp all_objects
	$(CXX) $(FLAGS) -o $(OUTPUT_DIR)/$(PROGNAME) console.cpp $(ALLOBJ)

build_test: test.cpp all_objects
	$(CXX) $(FLAGS) $(DOCTEST) -o $(OUTPUT_DIR)/test test.cpp $(ALLOBJ)

build_test_pixello: test_pixello.cpp all_objects
	$(CXX) $(FLAGS) -o $(OUTPUT_DIR)/test_pixello test_pixello.cpp $(ALLOBJ) $(SDL)

test: build_test 
	$(OUTPUT_DIR)/test

test_pixello: build_test_pixello
	$(OUTPUT_DIR)/test_pixello

clean:
	rm -rf $(OUTPUT_DIR)/*.o $(PROGNAME) test