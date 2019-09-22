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


/********************************************************
 *                  ADDRESSING MODES                    *
 ********************************************************/
bool MOS6502::ACC() {
    return true;
}

bool MOS6502::IMM() {
    return true;
}

bool MOS6502::ABS() {
    return true;
}

bool MOS6502::ZPI() {
    return true;
}

bool MOS6502::ZPX() {
    return true;
}

bool MOS6502::ZPY() {
    return true;
}

bool MOS6502::ABX() {
    return true;
}

bool MOS6502::ABY() {
    return true;
}

bool MOS6502::IMP() {
    return true;
}

bool MOS6502::REL() {
    return true;
}

bool MOS6502::IIX() {
    return true;
}

bool MOS6502::IIY() {
    return true;
}

bool MOS6502::IND() {
    return true;
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
