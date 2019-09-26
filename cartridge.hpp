#pragma once
#include <string>
#include <vector>
#include <stdint.h>
#include "mapper.hpp"

class Cartridge {
  private:
    enum mirror_t {
        HORIZONTAL,
        VERTICAL,
        ONESCREEN_LO,
        ONESCREEN_HI,
    };

    bool valid;

    std::vector<uint8_t> prg_memory;
    std::vector<uint8_t> chr_memory;
    uint8_t prg_banks;
    uint8_t chr_banks;

    uint8_t mapper_id;
    mirror_t mirror;

    Mapper *mapper;

  public:
    Cartridge(const std::string &file);
    ~Cartridge();

    bool is_valid();

    bool cpu_mem_access(uint16_t addr, access_mode_t access, uint8_t &data);
    bool ppu_mem_access(uint16_t addr, access_mode_t access, uint8_t &data);
};