#include "log.hpp"
#include "mos6502.hpp"


MOS6502::MOS6502(Bus *b) :
    A(0x00), X(0x00), Y(0x00), PC(0x0000), S(0x00), P(0x00), bus(b), cycles_count(0), cycles_needed(0) {

    if (bus == nullptr) {
        log_e("Bus is nullptr");
    }
}


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


void MOS6502::mem_fetch() {
    bus->access(cur_abb_add, Bus::READ, fetched);
}


void MOS6502::mem_write(uint16_t address, uint8_t data) {
    bus->access(address, Bus::WRITE, data);
}


void MOS6502::clock() {
    if (cycles_count == cycles_needed) {
        cur_abb_add = PC;
        mem_fetch();
        opcode = fetched;
        PC++;

        cycles_needed = opcode_table[opcode].cycles;
        cycles_count = 1;
    } else {
        (this->*opcode_table[opcode].addrmode)();
        (this->*opcode_table[opcode].operation)();
    }

    cycles_count++;
}


/********************************************************
 *                  ADDRESSING MODES                    *
 ********************************************************/
bool MOS6502::ACC() {   // DONE
    fetched = A;
    return false;
}

bool MOS6502::IMM() {   // DONE
    cur_abb_add = PC++;
    mem_fetch();
    return false;
}

bool MOS6502::ABS() {   // DONE
    cur_abb_add = PC++;
    mem_fetch();
    uint16_t tmp = fetched & 0x00FF;
    cur_abb_add = PC++;
    mem_fetch();
    cur_abb_add = (fetched << 8) | tmp;
    return false;
}

bool MOS6502::ZPI() {   // DONE
    cur_abb_add = PC++;
    mem_fetch();
    cur_abb_add = fetched & 0x00FF;
    return false;
}

bool MOS6502::ZPX() {   // DONE
    cur_abb_add = PC++;
    mem_fetch();
    cur_abb_add = (fetched + X) & 0x00FF;
    return false;
}

bool MOS6502::ZPY() {   // DONE
    cur_abb_add = PC++;
    mem_fetch();
    cur_abb_add = (fetched + Y) & 0x00FF;
    return false;
}

bool MOS6502::ABX() {   // DONE
    cur_abb_add = PC++;
    mem_fetch();
    uint16_t tmp = fetched & 0x00FF;
    cur_abb_add = PC++;
    mem_fetch();
    cur_abb_add = ((fetched << 8) | tmp) + X;

    if ((cur_abb_add & 0xFF00) != (fetched << 8)) {
        return true;
    }

    return false;
}

bool MOS6502::ABY() {   // DONE
    cur_abb_add = PC++;
    mem_fetch();
    uint16_t tmp = fetched & 0x00FF;
    cur_abb_add = PC++;
    mem_fetch();
    cur_abb_add = ((fetched << 8) | tmp) + Y;

    if ((cur_abb_add & 0xFF00) != (fetched << 8)) {
        return true;
    }

    return false;
}

bool MOS6502::IMP() {   // DONE
    return false;
}

bool MOS6502::REL() {   // DONE
    cur_abb_add = PC++;
    mem_fetch();
    cur_rel_add = fetched & 0x00FF;

    if (cur_rel_add & 0x80) {   // if cur_rel_add >= 128
        cur_rel_add |= 0xFF00;  // the this is negative offset
    }

    return false;
}

bool MOS6502::IIX() {
    cur_abb_add = PC++;
    mem_fetch();
    cur_abb_add = ((uint16_t)fetched + (uint16_t)X) & 0x00FF;
    mem_fetch();
    uint16_t tmp = fetched & 0x00FF;
    cur_abb_add++;
    mem_fetch();
    cur_abb_add = ((((uint16_t)fetched) << 8) & 0xFF00) | tmp;

    return false;
}

bool MOS6502::IIY() {
    cur_abb_add = PC++;
    mem_fetch();
    cur_abb_add = fetched & 0x00FF;
    mem_fetch();
    uint16_t tmp = fetched & 0x00FF;
    cur_abb_add++;
    mem_fetch();
    cur_abb_add = (((((uint16_t)fetched) << 8) & 0xFF00) | tmp) + Y;

    if ((cur_abb_add & 0xFF00) != (fetched << 8)) {
        return true;
    }

    return false;
}

bool MOS6502::IND() {
    cur_abb_add = PC++;
    mem_fetch();
    uint16_t tmp = fetched & 0x00FF;
    cur_abb_add = PC++;
    mem_fetch();
    tmp = (((uint16_t)fetched) << 8) | tmp;

    cur_abb_add = tmp;
    mem_fetch();
    tmp = fetched & 0x00FF;

    if (tmp == 0x00FF) {    // Page boundary hardware bug
        cur_abb_add &= 0xFF00;
    } else { // Behave normally
        cur_abb_add++;
    }

    mem_fetch();
    cur_abb_add = (((uint16_t)fetched) << 8) | tmp;

    return false;
}


/********************************************************
 *                   INSTRUCTION SET                    *
 ********************************************************/
bool MOS6502::ADC() {
    return true;
}

bool MOS6502::AND() {
    return true;
}

bool MOS6502::ASL() {
    return true;
}

bool MOS6502::BCC() {
    return true;
}

bool MOS6502::BCS() {
    return true;
}

bool MOS6502::BEQ() {
    return true;
}

bool MOS6502::BIT() {
    return true;
}

bool MOS6502::BMI() {
    return true;
}

bool MOS6502::BNE() {
    return true;
}

bool MOS6502::BPL() {
    return true;
}

bool MOS6502::BRK() {
    return true;
}

bool MOS6502::BVC() {
    return true;
}

bool MOS6502::BVS() {
    return true;
}

bool MOS6502::CLC() {
    return true;
}

bool MOS6502::CLD() {
    return true;
}

bool MOS6502::CLI() {
    return true;
}

bool MOS6502::CLV() {
    return true;
}

bool MOS6502::CMP() {
    return true;
}

bool MOS6502::CPX() {
    return true;
}

bool MOS6502::CPY() {
    return true;
}

bool MOS6502::DEC() {
    return true;
}

bool MOS6502::DEX() {
    return true;
}

bool MOS6502::DEY() {
    return true;
}

bool MOS6502::EOR() {
    return true;
}

bool MOS6502::INC() {
    return true;
}

bool MOS6502::INX() {
    return true;
}

bool MOS6502::INY() {
    return true;
}

bool MOS6502::JMP() {
    return true;
}

bool MOS6502::JSR() {
    return true;
}

bool MOS6502::LDA() {
    return true;
}

bool MOS6502::LDX() {
    return true;
}

bool MOS6502::LDY() {
    return true;
}

bool MOS6502::LSR() {
    return true;
}

bool MOS6502::NOP() {
    return true;
}

bool MOS6502::ORA() {
    return true;
}

bool MOS6502::PHA() {
    return true;
}

bool MOS6502::PHP() {
    return true;
}

bool MOS6502::PLA() {
    return true;
}

bool MOS6502::PLP() {
    return true;
}

bool MOS6502::ROL() {
    return true;
}

bool MOS6502::ROR() {
    return true;
}

bool MOS6502::RTI() {
    return true;
}

bool MOS6502::RTS() {
    return true;
}

bool MOS6502::SBC() {
    return true;
}

bool MOS6502::SEC() {
    return true;
}

bool MOS6502::SED() {
    return true;
}

bool MOS6502::SEI() {
    return true;
}

bool MOS6502::STA() {
    return true;
}

bool MOS6502::STX() {
    return true;
}

bool MOS6502::STY() {
    return true;
}

bool MOS6502::TAX() {
    return true;
}

bool MOS6502::TAY() {
    return true;
}

bool MOS6502::TSX() {
    return true;
}

bool MOS6502::TXA() {
    return true;
}

bool MOS6502::TXS() {
    return true;
}

bool MOS6502::TYA() {
    return true;
}

bool MOS6502::XXX() {
    return true;
}
