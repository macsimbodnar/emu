#include "log.hpp"
#include "util.hpp"
#include "bus.hpp"


Bus::Bus(Cartridge *p_cartridge) : cartridge(p_cartridge) {
    for (auto &I : RAM) {
        I = 0x00;
    }
}


void Bus::access(const uint16_t address, const access_mode_t read_write, uint8_t &data) {

    if (cartridge->cpu_mem_access(address, read_write, data)) {
        // The cartridge "sees all" and has the facility to veto
        // the propagation of the bus transaction if it requires.
        // This allows the cartridge to map any address to some
        // other data, including the facility to divert transactions
        // with other physical devices. The NES does not do this
        // but I figured it might be quite a flexible way of adding
        // "custom" hardware to the NES in the future!
    } else if (address >= 0x0000 && address <= 0x1FFF) {
        // System RAM Address Range. The range covers 8KB, though
        // there is only 2KB available. That 2KB is "mirrored"
        // through this address range. Using bitwise AND to mask
        // the bottom 11 bits is the same as addr % 2048.
        switch (read_write) {
        case access_mode_t::READ:
            data = RAM[address];
            break;

        case access_mode_t::WRITE:
            RAM[address] = data;
            break;

        default:
            log_e("Unexpected memory access type");
            break;
        }

    } else if (address >= 0x2000 && address <= 0x3FFF) {
        // PPU Address range. The PPU only has 8 primary registers
        // and these are repeated throughout this range. We can
        // use bitwise AND operation to mask the bottom 3 bits,
        // which is the equivalent of addr % 8.
        //ppu.cpuWrite(addr & 0x0007, data);
        log_e("ppu access");
    }
}


size_t Bus::get_mem_size() {
    return RAM.size();
}


uint8_t *Bus::get_mem_ptr() {
    return &(RAM[0]);
}