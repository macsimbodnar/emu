
pixello.o: pixello.cpp pixello.hpp
	$(CXX) $(FLAGS) -c -o pixello.o pixello.cpp $(SDL)

build_test_pixello: test_pixello.cpp $(ALLOBJ)
	$(CXX) $(FLAGS) -o test_pixello test_pixello.cpp $(ALLOBJ) $(SDL)

test_pixello: build_test_pixello
	./test_pixello

clean:
	rm -rf *.o test_pixello