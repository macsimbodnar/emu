
FLAGS := -Ilibs/linux/SDL2/include
ALLOBJ := pixello.o
SDL2 := -lSDL2

all: pixello.o

pixello.o: pixello.cpp pixello.hpp
	$(CXX) $(FLAGS) -c -o pixello.o pixello.cpp

build_test_pixello: test_pixello.cpp $(ALLOBJ)
	$(CXX) $(FLAGS) -o test_pixello test_pixello.cpp $(ALLOBJ) $(SDL2)

test: build_test_pixello
	./test_pixello

clean:
	rm -rf *.o test_pixello