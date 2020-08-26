#include "pixello.hpp"
#include <iostream>

void log(const std::string &msg) {
    std::cout << msg << std::endl;
}


int WinMain() {
    Pixello pixel;

    pixel.set_logger(log);

    pixel.init("Pixello Test", 1000, 500);

    pixel.close();

    return 0;
}