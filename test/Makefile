TEST_BIN = test

MAIN_DIR = ..
INCLUDES = -I $(MAIN_DIR)

OBJS = $(MAIN_DIR)/bus.o $(MAIN_DIR)/mos6502.o $(MAIN_DIR)/cartridge.o $(MAIN_DIR)/log.o $(MAIN_DIR)/mapper_000.o $(MAIN_DIR)/opcode.o $(MAIN_DIR)/console.o $(MAIN_DIR)/util.o

FLAGS = -g -O0 -Wall -Werror


build_test: $(OBJS) test.cpp 
	$(CXX) $(FLAGS) $(INCLUDES) -o $(TEST_BIN) test.cpp $(OBJS)

test: build_test
	./test

clean:
	$(MAKE) clean -C $(MAIN_DIR)
	rm -rf test