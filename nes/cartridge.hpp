#pragma once
#include <cstdint>
#include <vector>
#include <string>



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


  public:
    bool load_cartridge(const std::string &cartridge_file);

    uint8_t cpu_read(const uint16_t address);
    void cpu_write(const uint16_t address, const uint8_t data);

    uint8_t ppu_read(const uint16_t address);
    void ppu_write(const uint16_t address, const uint8_t data);
};