#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include "bus.hpp"
#include "mos6502.hpp"
#include "log.hpp"


#define TEST_BIN "documentation/nestest.nes"
#define LOAD_MEMORY_IN  0x0A000
#define HIGH_BYTE       0x0A
#define LOW_BYTE        0x00

#define RESULT_LOCATION_1 0x0002
#define RESULT_LOCATION_2 0x0003

TEST_CASE("Test") {

}

// 2b current PC | 1b opcode | arg 1 | arg2 | menomonic (es JMP $(C5F5)) |       A:xx | X:xx | Y:xx | P:xx | SP:xx | PPU: xxx, 0 | CYC:dec