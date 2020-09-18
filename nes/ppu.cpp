#include "ppu.hpp"


PPU::PPU() : cartridge(nullptr) {}


uint8_t PPU::cpu_read(const uint16_t address, const bool read_only) {
    uint8_t data = 0x00;

    switch (address) {
    case 0x0000: // Controll
        break;

    case 0x0001: // Mask
        break;

    case 0x0002: // Status
        break;

    case 0x0003: // OAM Address
        break;

    case 0x0004: // OAM Data
        break;

    case 0x0005: // Scroll
        break;

    case 0x0006: // PPU Address
        break;

    case 0x0007: // PPU Data
        break;

    default:
        // We should never go there
        break;
    }

    return data;
}


void PPU::cpu_write(const uint16_t address, const uint8_t data) {
    switch (address) {
    case 0x0000: // Controll
        break;

    case 0x0001: // Mask
        break;

    case 0x0002: // Status
        break;

    case 0x0003: // OAM Address
        break;

    case 0x0004: // OAM Data
        break;

    case 0x0005: // Scroll
        break;

    case 0x0006: // PPU Address
        break;

    case 0x0007: // PPU Data
        break;

    default:
        // We should never go there
        break;
    }
}


uint8_t PPU::ppu_read(const uint16_t address, const bool read_only) {
    uint8_t data = 0x00;
    const uint16_t masked_address = address & 0x3FFF;

    // TODO(max): should check if the cartridge is not nullptr
    if (cartridge->ppu_read(masked_address, data)) {
        // DO NOTHING
    }

    return data;
}


void PPU::ppu_write(const uint16_t address, const uint8_t data) {
    const uint16_t masked_address = address & 0x3FFF;

    if (cartridge->ppu_write(masked_address, data)) {
        // DO NOTHING
    }
}


void PPU::connect_cartridge(Cartridge *cart) {
    cartridge = cart;
}