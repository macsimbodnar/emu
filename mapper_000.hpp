#pragma once
#include <stdint.h>
#include "mapper.hpp"

class Mapper_000 : public Mapper {
  public:
    Mapper_000(uint8_t prg_banks, uint8_t chr_banks);

    // Transform CPU bus address into PRG ROM offset
    bool cpu_address(uint16_t addr, access_mode_t access, uint32_t &address_out) override;
    // Transform PPU bus address into CHR ROM offset
    bool ppu_address(uint16_t addr, access_mode_t access, uint32_t &address_out) override;
};