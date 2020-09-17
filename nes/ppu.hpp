#pragma once

/*******************************************************************************
 *
 * 0x0000 - 0x1FFF          Pattern Memory Char ROM         Sprites
 * 0x2000 - 0x3EFF          Name Table memory (VRAM)        Layout of background
 * 0x3F00 - 0x3FFF          Palette Memory                  Colors
 *
 *                     0    <-16->      4               8
 *                     ----------------------------------
 *                     |                |        8      |
 *                     |                |      -----    |
 *                  16 |                |     8|   |    |
 *       128 pixels    |                |      -----    |
 *                     |                |     pixels    |
 *                     |                |               |
 *                     ----------------------------------
 *                       <-128 pixels->
 *
 *
 * 2C02
 *******************************************************************************/
#include <cstdint>
#include "cartridge.hpp"

class PPU {
    Cartridge *cartridge;

    uint8_t name_table[2][1024];  // 2KBytes of VRAM for name table
    uint8_t palette[32];

  public:
    PPU();

    uint8_t cpu_read(const uint16_t address, const bool read_only);
    void cpu_write(const uint16_t address, const uint8_t data);

    uint8_t ppu_read(const uint16_t address, const bool read_only);
    void ppu_write(const uint16_t address, const uint8_t data);

    void connect_cartridge(Cartridge *cartridge);
    void clock();
};