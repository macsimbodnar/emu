#include "log.hpp"
#include "mapper_000.hpp"


Mapper_000::Mapper_000(uint8_t prg_banks, uint8_t chr_banks) : Mapper(prg_banks, chr_banks) {}


bool Mapper_000::cpu_address(uint16_t addr, access_mode_t access, uint32_t &address_out) {
    switch (access) {
    case access_mode_t::READ:
    case access_mode_t::WRITE:

        // if PRG ROM is 16KB
        //     CPU Address Bus          PRG ROM
        //     0x8000 -> 0xBFFF: Map    0x0000 -> 0x3FFF
        //     0xC000 -> 0xFFFF: Mirror 0x0000 -> 0x3FFF
        // if PRG ROM is 32KB
        //     CPU Address Bus          PRG ROM
        //     0x8000 -> 0xFFFF: Map    0x0000 -> 0x7FFF
        if (addr >= 0x8000 && addr <= 0xFFFF) {
            address_out = addr & (prg_banks > 1 ? 0x7FFF : 0x3FFF);
            return true;
        }

        break;

    default:
        log_e("Mapper_000 using unexpected access mode");
        break;
    }

    return false;
}


bool Mapper_000::ppu_address(uint16_t addr, access_mode_t access, uint32_t &address_out) {
    switch (access) {
    case access_mode_t::READ:

        // There is no mapping required for PPU
        // PPU Address Bus          CHR ROM
        // 0x0000 -> 0x1FFF: Map    0x0000 -> 0x1FFF
        if (addr >= 0x0000 && addr <= 0x1FFF) {
            address_out = addr;
            return true;
        }

        break;

    case access_mode_t::WRITE:
        if (addr >= 0x0000 && addr <= 0x1FFF) {
            if (chr_banks == 0) {
                // Treat as RAM
                address_out = addr;
                return true;
            }
        }

        break;

    default:
        log_e("Mapper_000 using unexpected access mode");
        break;
    }

    return false;
}

