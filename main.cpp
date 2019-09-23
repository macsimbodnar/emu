#include "mos6502.hpp"
#include "bus.hpp"
#include <iostream>

int main(int argc, char *argv[]) {
    Bus b;
    MOS6502 cpu(&b);

    return 0;
}