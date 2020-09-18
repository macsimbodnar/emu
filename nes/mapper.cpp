#include "mapper.hpp"


Mapper::Mapper(const uint8_t _PRG_banks, const uint8_t _CHR_banks) {
    PRG_banks = _PRG_banks;
    CHR_banks = _CHR_banks;
}


Mapper_000::Mapper_000(const uint8_t PRG_banks, const uint8_t CHR_banks) : Mapper(PRG_banks, CHR_banks) {}


bool Mapper_000::cpu_map_read(const uint16_t address, uint32_t &mapped_address) {

    if (address >= 0x8000 && address <= 0xFFFF) {
        mapped_address = address & (PRG_banks > 1 ? 0x7FFF : 0x3FFF);
        return true;
    }

    return false;
}


bool Mapper_000::cpu_map_write(const uint16_t address, uint32_t &mapped_address) {

    if (address >= 0x8000 && address <= 0xFFFF) {
        mapped_address = address & (PRG_banks > 1 ? 0x7FFF : 0x3FFF);
        return true;
    }

    return false;
}


bool Mapper_000::ppu_map_read(const uint16_t address, uint32_t &mapped_address) {

    if (address >= 0x0000 && address <= 0x1FFF) {
        mapped_address = address;
        return true;
    }

    return false;
}


bool Mapper_000::ppu_map_write(const uint16_t address, uint32_t &mapped_address) {

    // if (address >= 0x0000 && address <= 0x1FFF) {

    //     return true;
    // }

    return false;
}

