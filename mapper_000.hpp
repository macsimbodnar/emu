#pragma once
#include <stdint.h>
#include "mapper.hpp"

class Mapper_000 : private Mapper {
  public:
    Mapper_000(uint8_t prg_banks, uint8_t chr_banks);

    // Transform CPU bus address into PRG ROM offset
    uint32_t cpu_address(uint16_t addr, access_mode_t access) override;
    // Transform PPU bus address into CHR ROM offset
    uint32_t ppu_address(uint16_t addr, access_mode_t access) override;
};