#include "log.hpp"
#include "util.hpp"
#include "bus.hpp"


Bus::Bus() {
    for (auto &I : RAM) {
        I = 0x00;
    }
}


void Bus::access(const uint16_t address, const access_mode_t read_write, uint8_t &data) {

    if (address < MIN_ADDRESS || address > MAX_ADDRESS) {
        log_e("Address out of bounds " + uint16_to_hex(address, true));
        return;
    }

    switch (read_write) {

    case access_mode_t::READ:
        data =  RAM[address];
        break;

    case access_mode_t::WRITE:
        RAM[address] = data;
        break;

    default:
        log_e("Wrong access type to the address " + uint16_to_hex(address, true));
    }
}


size_t Bus::gem_mem_size() {
    return RAM.size();
}


uint8_t *Bus::get_mem_ptr() {
    return &(RAM[0]);
}