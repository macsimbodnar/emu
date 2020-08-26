#include <iostream>
#include <thread>
#include "pixello.hpp"

void log(const std::string &msg) {
    std::cout << msg << std::endl;
}


void update(unsigned char *pixels,
            const unsigned int w,
            const unsigned int h,
            const unsigned int c) {

    for (unsigned int i = 0; i < 1000; i++) {
        const unsigned int x = rand() % w;
        const unsigned int y = rand() % h;

        const unsigned int offset = (w * c * y) + x * c;
        pixels[ offset + 0 ] = rand() % 256;        // b
        pixels[ offset + 1 ] = rand() % 256;        // g
        pixels[ offset + 2 ] = rand() % 256;        // r
        pixels[ offset + 3 ] = SDL_ALPHA_OPAQUE;    // a
    }
}


int WinMain() {
    set_logger(&log);

    if (!init()) {
        return 1;
    }

    if (!create_window("Pixello Test", 1280, 720)) {
        return 2;
    }

    if (!run(&update, 20)) {
        return 3;
    }

    close();

    return 0;
}