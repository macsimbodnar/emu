#pragma once

#include <array>
#include "cartridge.hpp"
#include "ppu.hpp"
#include "cpu.hpp"


/**
 * NES:
 * RAM:     0x0000 - 0x1FFF
 *          The 8KBytes of ram is mirored to the first 2KBytes
 *
 * PPU:     0x0000 - 0x1FFF     8KBytes Patter
 *          0x2000 - 0x2FFF     2KBytes Nametable
 *          0x3F00 - 0x3FFF             Palets
 *
 * ROM:     0x4020 - 0xFFFF
 */
class NES {
  private:
    uint32_t n_system_clock_counter;

    std::array<uint8_t, 2 * 1024> cpu_RAM;   // 2KBytes of RAM fro the CPU
    PPU ppu;
    CPU cpu;

    void cpu_write(const uint16_t address, const uint8_t data);
    uint8_t cpu_read(const uint16_t address, const bool read_only);


  public:
    NES();

    void insert_cartridge(const Cartridge *cartridge);
    void reset();
    void clock();
};