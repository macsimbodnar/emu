#include "nes.hpp"


NES::NES() {
    n_system_clock_counter = 0;
}


uint8_t NES::cpu_read(const uint16_t address, const bool read_only) {
    uint8_t data = 0x00;

    if (address >= 0x0000 && address <= 0x1FFF) {
        data = cpu_RAM[address & 0x07FF]; // Mirroring the RAM
    }
    // PPU
    else if (address >= 0x2000 && address <= 0x3FFF) {
        data = ppu.cpu_read(address & 0x0007, read_only);
    }

    return data;
}


void NES::cpu_write(const uint16_t address, const uint8_t data) {

    if (address >= 0x0000 && address <= 0x1FFF) {
        cpu_RAM[address & 0x07FF] = data; // Mirroring the RAM
    }
    // PPU
    else if (address >= 0x2000 && address <= 0x3FFF) {
        ppu.cpu_write(address & 0x0007, data);
    }
}


void NES::reset() {
    // cpu.reset();
    n_system_clock_counter = 0;
}

