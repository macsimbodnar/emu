#pragma once
#include <cstdint>


class Mapper {
  protected:
    uint8_t PRG_banks = 0;
    uint8_t CHR_banks = 0;

  public:
    Mapper(const uint8_t PRG_banks, const uint8_t CHR_banks);

    virtual bool cpu_map_read(const uint16_t address, uint32_t &mapped_address) = 0;
    virtual bool cpu_map_write(const uint16_t address, uint32_t &mapped_address) = 0;

    virtual bool ppu_map_read(const uint16_t address, uint32_t &mapped_address) = 0;
    virtual bool ppu_map_write(const uint16_t address, uint32_t &mapped_address) = 0;
};


class Mapper_000 : public Mapper {

public:
    Mapper_000(const uint8_t PRG_banks, const uint8_t CHR_banks);

    bool cpu_map_read(const uint16_t address, uint32_t &mapped_address) override;
    bool cpu_map_write(const uint16_t address, uint32_t &mapped_address) override;

    bool ppu_map_read(const uint16_t address, uint32_t &mapped_address) override;
    bool ppu_map_write(const uint16_t address, uint32_t &mapped_address) override;
};