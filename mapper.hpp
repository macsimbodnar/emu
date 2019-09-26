#pragma once
#include <stdint.h>
#include "common.hpp"


class Mapper {
  public:
    Mapper(uint8_t prg_b, uint8_t chr_b) : prg_banks(prg_b), chr_banks(chr_b) {}

    uint8_t prg_banks;
    uint8_t chr_banks;

    // Transform CPU bus address into PRG ROM offset
    virtual bool cpu_address(uint16_t addr, access_mode_t access, uint32_t &address_out) = 0;
    // Transform PPU bus address into CHR ROM offset
    virtual bool ppu_address(uint16_t addr, access_mode_t access, uint32_t &address_out) = 0;
};