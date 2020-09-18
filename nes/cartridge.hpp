#pragma once
#include <cstdint>
#include <vector>
#include <string>
#include "mapper.hpp"


class Cartridge {
  private:

    enum class mirror_t {
        HORIZONTAL,
        VERTICAL,
        ONESCREEN_LO,
        ONESCREEN_HI,
    };

    std::vector<uint8_t> PRG_memory;    // CPU Program memory
    std::vector<uint8_t> CHR_memory;    // PPU Character memory
    uint8_t PRG_banks;
    uint8_t CHR_banks;
    uint8_t mapper_id;
    mirror_t mirror;

    Mapper *mapper = nullptr;

  public:
    bool load_cartridge(const std::string &cartridge_file);

    bool cpu_read(const uint16_t address, uint8_t &data);
    bool cpu_write(const uint16_t address, const uint8_t data);

    bool ppu_read(const uint16_t address, uint8_t &data);
    bool ppu_write(const uint16_t address, const uint8_t data);
};