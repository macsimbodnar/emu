#include "mos6502.hpp"

MOS6502::MOS6502(Bus *b) : bus(b) {}


void MOS6502::set_flag(const status_flag_t flag, const bool val) {
    if (val) {
        S |= flag;
    } else {
        S &= ~flag;
    }
}


bool MOS6502::read_flag(const status_flag_t flag) {
    return (S & flag);
}


void MOS6502::clock() {
    
}