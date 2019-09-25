#include "bus.hpp"
#include "mos6502.hpp"
#include "log.hpp"

#define TEST_BIN "documentation/nestest.nes"
#define LOAD_MEMORY_IN  0x0A000
#define HIGH_BYTE       0x0A
#define LOW_BYTE        0x00

#define RESULT_LOCATION_1 0x0002
#define RESULT_LOCATION_2 0x0003

int main() {
    Bus b;
    MOS6502 cpu(&b);
    FILE *file;
    long size;
    uint8_t *mem_ptr = b.get_mem_ptr();

    file = fopen(TEST_BIN, "rb");

    if (file == nullptr) {
        log_e("Can not open the file " + std::string(TEST_BIN));
        exit(EXIT_FAILURE);
    }

    // get the file size
    fseek(file, 0, SEEK_END);
    size = ftell(file);
    fseek(file, 0, SEEK_SET);

    uint8_t *mem_ptr_off = mem_ptr + LOAD_MEMORY_IN;

    if (fread(mem_ptr_off, sizeof(uint8_t), size, file) != (size_t)size) {
        log_e("Failed read instructions from file");
        exit(EXIT_FAILURE);
    }

    mem_ptr[0xFFFC] = LOW_BYTE;                     // Set the reset Vector
    mem_ptr[0xFFFD] = HIGH_BYTE;

    cpu.reset();

    while (true) {
        cpu.clock();
    }

    exit(EXIT_SUCCESS);
}