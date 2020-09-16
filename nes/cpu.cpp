#include "cpu.hpp"


void CPU::mem_callback(void *usr_data, const uint16_t address, const access_mode_t read_write, uint8_t &data) {
    // CPU *cpu = static_cast<CPU *>(usr_data);

    switch (read_write) {
    case access_mode_t::READ:
        // data = cpu_read(address, false);
        break;

    case access_mode_t::WRITE:
        // cpu_write(address, data);
        break;

    case access_mode_t::READ_ONLY:
        break;
    }
}


CPU::CPU() : cpu(mem_callback, this) {}