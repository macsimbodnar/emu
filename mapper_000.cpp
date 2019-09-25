#include "mapper_000.hpp"


Mapper_000::Mapper_000(uint8_t prg_banks, uint8_t chr_banks) : Mapper(prg_banks, chr_banks) {}


uint32_t Mapper_000::cpu_address(uint16_t addr, access_mode_t access) {
    switch (access) {
    case access_mode_t::READ:
        break;

    case access_mode_t::WRITE:
        break;

    default:
        break;
    }
}


uint32_t Mapper_000::ppu_address(uint16_t addr, access_mode_t access) {
    switch (access) {
    case access_mode_t::READ:
        break;

    case access_mode_t::WRITE:
        break;

    default:
        break;
    }
}

