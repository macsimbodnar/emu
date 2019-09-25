#pragma once
#include <stdint.h>
#include "common.hpp"


class Mapper {
  private:
    uint8_t prg_banks;
    uint8_t chr_banks;

  public:
    Mapper(uint8_t prg_b, uint8_t chr_b) : prg_banks(prg_b), chr_banks(chr_b) {}

    // Transform CPU bus address into PRG ROM offset
    virtual uint32_t cpu_address(uint16_t addr, access_mode_t access) = 0;
    // Transform PPU bus address into CHR ROM offset
    virtual uint32_t ppu_address(uint16_t addr, access_mode_t access) = 0;
};