#include "mos6502.hpp"

using M = MOS6502;

// TODO(max): fix the illegal (*) opcode cycles numbers according to:
// https://wiki.nesdev.com/w/index.php/Programming_with_unofficial_opcodes

// clang-format off
const std::vector<MOS6502::instruction_t> MOS6502::opcode_table = {
    //                    0                                  1                                  2                                    3                                   4                                        5                                  6                                   7                                  8                                  9                                         A                                   B                                   C                                  D                                  E                                   F
    /*0*/ { "BRK",  &M::BRK, &M::IMP, 7, 1 }, { "ORA", &M::ORA, &M::IIX, 6, 2 }, { "???", &M::XXX, &M::IMP, 2, 0 }, { "*SLO",  &M::SLO, &M::IIX, 8, 2 }, { "*NOP", &M::NO2, &M::IMM, 3, 2 }, /*0*/ { "ORA", &M::ORA, &M::ZPI, 3, 2 }, { "ASL", &M::ASL, &M::ZPI, 5, 2 }, { "*SLO", &M::SLO, &M::ZPI, 5, 2 }, { "PHP", &M::PHP, &M::IMP, 3, 1 }, { "ORA", &M::ORA, &M::IMM, 2, 2 }, /*0*/ { "ASL",  &M::ASL, &M::ACC, 2, 1 }, { "???",  &M::XXX, &M::IMP, 2, 0 }, { "*NOP", &M::NOP, &M::ABS, 4, 3 }, { "ORA", &M::ORA, &M::ABS, 4, 3 }, { "ASL", &M::ASL, &M::ABS, 6, 3 }, { "*SLO", &M::SLO, &M::ABS, 6, 3 }, /*0*/
    /*1*/ { "BPL",  &M::BPL, &M::REL, 2, 2 }, { "ORA", &M::ORA, &M::IIY, 5, 2 }, { "???", &M::XXX, &M::IMP, 2, 0 }, { "*SLO",  &M::SLO, &M::IIY, 8, 2 }, { "*NOP", &M::NOP, &M::ZPX, 4, 2 }, /*1*/ { "ORA", &M::ORA, &M::ZPX, 4, 2 }, { "ASL", &M::ASL, &M::ZPX, 6, 2 }, { "*SLO", &M::SLO, &M::ZPX, 6, 2 }, { "CLC", &M::CLC, &M::IMP, 2, 1 }, { "ORA", &M::ORA, &M::ABY, 4, 3 }, /*1*/ { "*NOP", &M::NOP, &M::IMP, 2, 1 }, { "*SLO", &M::SLO, &M::ABY, 7, 3 }, { "*NOP", &M::NOP, &M::ABX, 2, 3 }, { "ORA", &M::ORA, &M::ABX, 4, 3 }, { "ASL", &M::ASL, &M::ABX, 7, 3 }, { "*SLO", &M::SLO, &M::ABX, 7, 3 }, /*1*/
    /*2*/ { "JSR",  &M::JSR, &M::ABS, 6, 3 }, { "AND", &M::AND, &M::IIX, 6, 2 }, { "???", &M::XXX, &M::IMP, 2, 0 }, { "*RLA",  &M::RLA, &M::IIX, 8, 2 }, { "BIT",  &M::BIT, &M::ZPI, 3, 2 }, /*2*/ { "AND", &M::AND, &M::ZPI, 3, 2 }, { "ROL", &M::ROL, &M::ZPI, 5, 2 }, { "*RLA", &M::RLA, &M::ZPI, 5, 2 }, { "PLP", &M::PLP, &M::IMP, 4, 1 }, { "AND", &M::AND, &M::IMM, 2, 2 }, /*2*/ { "ROL",  &M::ROL, &M::ACC, 2, 1 }, { "???",  &M::XXX, &M::IMP, 2, 0 }, { "BIT",  &M::BIT, &M::ABS, 4, 3 }, { "AND", &M::AND, &M::ABS, 4, 3 }, { "ROL", &M::ROL, &M::ABS, 6, 3 }, { "*RLA", &M::RLA, &M::ABS, 6, 3 }, /*2*/
    /*3*/ { "BMI",  &M::BMI, &M::REL, 2, 2 }, { "AND", &M::AND, &M::IIY, 5, 2 }, { "???", &M::XXX, &M::IMP, 2, 0 }, { "*RLA",  &M::RLA, &M::IIY, 8, 2 }, { "*NOP", &M::NO2, &M::ZPI, 3, 2 }, /*3*/ { "AND", &M::AND, &M::ZPX, 4, 2 }, { "ROL", &M::ROL, &M::ZPX, 6, 2 }, { "*RLA", &M::RLA, &M::ZPX, 6, 2 }, { "SEC", &M::SEC, &M::IMP, 2, 1 }, { "AND", &M::AND, &M::ABY, 4, 3 }, /*3*/ { "*NOP", &M::NOP, &M::IMP, 2, 1 }, { "*RLA", &M::RLA, &M::ABY, 7, 3 }, { "*NOP", &M::NOP, &M::ABX, 2, 3 }, { "AND", &M::AND, &M::ABX, 4, 3 }, { "ROL", &M::ROL, &M::ABX, 7, 3 }, { "*RLA", &M::RLA, &M::ABX, 7, 3 }, /*3*/
    /*4*/ { "RTI",  &M::RTI, &M::IMP, 6, 1 }, { "EOR", &M::EOR, &M::IIX, 6, 2 }, { "???", &M::XXX, &M::IMP, 2, 0 }, { "*SRE",  &M::SRE, &M::IIX, 8, 2 }, { "*NOP", &M::NO2, &M::IMM, 3, 2 }, /*4*/ { "EOR", &M::EOR, &M::ZPI, 3, 2 }, { "LSR", &M::LSR, &M::ZPI, 5, 2 }, { "*SRE", &M::SRE, &M::ZPI, 5, 2 }, { "PHA", &M::PHA, &M::IMP, 3, 1 }, { "EOR", &M::EOR, &M::IMM, 2, 2 }, /*4*/ { "LSR",  &M::LSR, &M::ACC, 2, 1 }, { "???",  &M::XXX, &M::IMP, 2, 0 }, { "JMP",  &M::JMP, &M::ABS, 3, 3 }, { "EOR", &M::EOR, &M::ABS, 4, 3 }, { "LSR", &M::LSR, &M::ABS, 6, 3 }, { "*SRE", &M::SRE, &M::ABS, 6, 3 }, /*4*/
    /*5*/ { "BVC",  &M::BVC, &M::REL, 2, 2 }, { "EOR", &M::EOR, &M::IIY, 5, 2 }, { "???", &M::XXX, &M::IMP, 2, 0 }, { "*SRE",  &M::SRE, &M::IIY, 8, 2 }, { "*NOP", &M::NO2, &M::ZPI, 3, 2 }, /*5*/ { "EOR", &M::EOR, &M::ZPX, 4, 2 }, { "LSR", &M::LSR, &M::ZPX, 6, 2 }, { "*SRE", &M::SRE, &M::ZPX, 6, 2 }, { "CLI", &M::CLI, &M::IMP, 2, 1 }, { "EOR", &M::EOR, &M::ABY, 4, 3 }, /*5*/ { "*NOP", &M::NOP, &M::IMP, 2, 1 }, { "*SRE", &M::SRE, &M::ABY, 7, 3 }, { "*NOP", &M::NOP, &M::ABX, 2, 3 }, { "EOR", &M::EOR, &M::ABX, 4, 3 }, { "LSR", &M::LSR, &M::ABX, 7, 3 }, { "*SRE", &M::SRE, &M::ABX, 7, 3 }, /*5*/
    /*6*/ { "RTS",  &M::RTS, &M::IMP, 6, 1 }, { "ADC", &M::ADC, &M::IIX, 6, 2 }, { "???", &M::XXX, &M::IMP, 2, 0 }, { "*RRA",  &M::RRA, &M::IIX, 8, 2 }, { "*NOP", &M::NO2, &M::IMM, 3, 2 }, /*6*/ { "ADC", &M::ADC, &M::ZPI, 3, 2 }, { "ROR", &M::ROR, &M::ZPI, 5, 2 }, { "*RRA", &M::RRA, &M::ZPI, 5, 2 }, { "PLA", &M::PLA, &M::IMP, 4, 1 }, { "ADC", &M::ADC, &M::IMM, 2, 2 }, /*6*/ { "ROR",  &M::ROR, &M::ACC, 2, 1 }, { "???",  &M::XXX, &M::IMP, 2, 0 }, { "JMP",  &M::JMP, &M::IND, 5, 3 }, { "ADC", &M::ADC, &M::ABS, 4, 3 }, { "ROR", &M::ROR, &M::ABS, 6, 3 }, { "*RRA", &M::RRA, &M::ABS, 6, 3 }, /*6*/
    /*7*/ { "BVS",  &M::BVS, &M::REL, 2, 2 }, { "ADC", &M::ADC, &M::IIY, 5, 2 }, { "???", &M::XXX, &M::IMP, 2, 0 }, { "*RRA",  &M::RRA, &M::IIY, 8, 2 }, { "*NOP", &M::NO2, &M::ZPI, 3, 2 }, /*7*/ { "ADC", &M::ADC, &M::ZPX, 4, 2 }, { "ROR", &M::ROR, &M::ZPX, 6, 2 }, { "*RRA", &M::RRA, &M::ZPX, 6, 2 }, { "SEI", &M::SEI, &M::IMP, 2, 1 }, { "ADC", &M::ADC, &M::ABY, 4, 3 }, /*7*/ { "*NOP", &M::NOP, &M::IMP, 2, 1 }, { "*RRA", &M::RRA, &M::ABY, 7, 3 }, { "*NOP", &M::NOP, &M::ABX, 2, 3 }, { "ADC", &M::ADC, &M::ABX, 4, 3 }, { "ROR", &M::ROR, &M::ABX, 7, 3 }, { "*RRA", &M::RRA, &M::ABX, 7, 3 }, /*7*/
    /*8*/ { "*NOP", &M::NOP, &M::IMM, 2, 2 }, { "STA", &M::STA, &M::IIX, 6, 2 }, { "???", &M::NOP, &M::IMP, 2, 1 }, { "*SAX",  &M::SAX, &M::IIX, 6, 2 }, { "STY",  &M::STY, &M::ZPI, 3, 2 }, /*8*/ { "STA", &M::STA, &M::ZPI, 3, 2 }, { "STX", &M::STX, &M::ZPI, 3, 2 }, { "*SAX", &M::SAX, &M::ZPI, 3, 2 }, { "DEY", &M::DEY, &M::IMP, 2, 1 }, { "???", &M::NOP, &M::IMP, 2, 1 }, /*8*/ { "TXA",  &M::TXA, &M::IMP, 2, 1 }, { "???",  &M::XXX, &M::IMP, 2, 0 }, { "STY",  &M::STY, &M::ABS, 4, 3 }, { "STA", &M::STA, &M::ABS, 4, 3 }, { "STX", &M::STX, &M::ABS, 4, 3 }, { "*SAX", &M::SAX, &M::ABS, 4, 3 }, /*8*/
    /*9*/ { "BCC",  &M::BCC, &M::REL, 2, 2 }, { "STA", &M::STA, &M::IIY, 6, 2 }, { "???", &M::XXX, &M::IMP, 2, 0 }, { "???",   &M::XXX, &M::IMP, 6, 0 }, { "STY",  &M::STY, &M::ZPX, 4, 2 }, /*9*/ { "STA", &M::STA, &M::ZPX, 4, 2 }, { "STX", &M::STX, &M::ZPY, 4, 2 }, { "*SAX", &M::SAX, &M::ZPY, 4, 2 }, { "TYA", &M::TYA, &M::IMP, 2, 1 }, { "STA", &M::STA, &M::ABY, 5, 3 }, /*9*/ { "TXS",  &M::TXS, &M::IMP, 2, 1 }, { "???",  &M::XXX, &M::IMP, 5, 0 }, { "???",  &M::NOP, &M::IMP, 5, 1 }, { "STA", &M::STA, &M::ABX, 5, 3 }, { "???", &M::XXX, &M::IMP, 5, 0 }, { "???",  &M::XXX, &M::IMP, 5, 0 }, /*9*/
    /*A*/ { "LDY",  &M::LDY, &M::IMM, 2, 2 }, { "LDA", &M::LDA, &M::IIX, 6, 2 }, { "LDX", &M::LDX, &M::IMM, 2, 2 }, { "*LAX",  &M::LAX, &M::IIX, 6, 2 }, { "LDY",  &M::LDY, &M::ZPI, 3, 2 }, /*A*/ { "LDA", &M::LDA, &M::ZPI, 3, 2 }, { "LDX", &M::LDX, &M::ZPI, 3, 2 }, { "*LAX", &M::LAX, &M::ZPI, 6, 2 }, { "TAY", &M::TAY, &M::IMP, 2, 1 }, { "LDA", &M::LDA, &M::IMM, 2, 2 }, /*A*/ { "TAX",  &M::TAX, &M::IMP, 2, 1 }, { "???",  &M::XXX, &M::IMP, 2, 0 }, { "LDY",  &M::LDY, &M::ABS, 4, 3 }, { "LDA", &M::LDA, &M::ABS, 4, 3 }, { "LDX", &M::LDX, &M::ABS, 4, 3 }, { "*LAX", &M::LAX, &M::ABS, 4, 3 }, /*A*/
    /*B*/ { "BCS",  &M::BCS, &M::REL, 2, 2 }, { "LDA", &M::LDA, &M::IIY, 5, 2 }, { "???", &M::XXX, &M::IMP, 2, 0 }, { "*LAX",  &M::LAX, &M::IIY, 5, 2 }, { "LDY",  &M::LDY, &M::ZPX, 4, 2 }, /*B*/ { "LDA", &M::LDA, &M::ZPX, 4, 2 }, { "LDX", &M::LDX, &M::ZPY, 4, 2 }, { "*LAX", &M::LAX, &M::ZPY, 4, 2 }, { "CLV", &M::CLV, &M::IMP, 2, 1 }, { "LDA", &M::LDA, &M::ABY, 4, 3 }, /*B*/ { "TSX",  &M::TSX, &M::IMP, 2, 1 }, { "???",  &M::XXX, &M::IMP, 4, 0 }, { "LDY",  &M::LDY, &M::ABX, 4, 3 }, { "LDA", &M::LDA, &M::ABX, 4, 3 }, { "LDX", &M::LDX, &M::ABY, 4, 3 }, { "*LAX", &M::LAX, &M::ABY, 4, 3 }, /*B*/
    /*C*/ { "CPY",  &M::CPY, &M::IMM, 2, 2 }, { "CMP", &M::CMP, &M::IIX, 6, 2 }, { "???", &M::NOP, &M::IMP, 2, 1 }, { "*DCP",  &M::DCP, &M::IIX, 6, 2 }, { "CPY",  &M::CPY, &M::ZPI, 3, 2 }, /*C*/ { "CMP", &M::CMP, &M::ZPI, 3, 2 }, { "DEC", &M::DEC, &M::ZPI, 5, 2 }, { "*DCP", &M::DCP, &M::ZPI, 5, 2 }, { "INY", &M::INY, &M::IMP, 2, 1 }, { "CMP", &M::CMP, &M::IMM, 2, 2 }, /*C*/ { "DEX",  &M::DEX, &M::IMP, 2, 1 }, { "???",  &M::XXX, &M::IMP, 2, 0 }, { "CPY",  &M::CPY, &M::ABS, 4, 3 }, { "CMP", &M::CMP, &M::ABS, 4, 3 }, { "DEC", &M::DEC, &M::ABS, 6, 3 }, { "*DCP", &M::DCP, &M::ABS, 6, 3 }, /*C*/
    /*D*/ { "BNE",  &M::BNE, &M::REL, 2, 2 }, { "CMP", &M::CMP, &M::IIY, 5, 2 }, { "???", &M::XXX, &M::IMP, 2, 0 }, { "*DCP",  &M::DCP, &M::IIY, 8, 2 }, { "*NOP", &M::NO2, &M::ZPI, 3, 2 }, /*D*/ { "CMP", &M::CMP, &M::ZPX, 4, 2 }, { "DEC", &M::DEC, &M::ZPX, 6, 2 }, { "*DCP", &M::DCP, &M::ZPX, 6, 2 }, { "CLD", &M::CLD, &M::IMP, 2, 1 }, { "CMP", &M::CMP, &M::ABY, 4, 3 }, /*D*/ { "*NOP", &M::NOP, &M::IMP, 2, 1 }, { "*DCP", &M::DCP, &M::ABY, 7, 3 }, { "*NOP", &M::NOP, &M::ABX, 2, 3 }, { "CMP", &M::CMP, &M::ABX, 4, 3 }, { "DEC", &M::DEC, &M::ABX, 7, 3 }, { "*DCP", &M::DCP, &M::ABX, 7, 3 }, /*D*/
    /*E*/ { "CPX",  &M::CPX, &M::IMM, 2, 2 }, { "SBC", &M::SBC, &M::IIX, 6, 2 }, { "???", &M::NOP, &M::IMP, 2, 1 }, { "*ISB",  &M::ISB, &M::IIX, 8, 2 }, { "CPX",  &M::CPX, &M::ZPI, 3, 2 }, /*E*/ { "SBC", &M::SBC, &M::ZPI, 3, 2 }, { "INC", &M::INC, &M::ZPI, 5, 2 }, { "*ISB", &M::ISB, &M::ZPI, 5, 2 }, { "INX", &M::INX, &M::IMP, 2, 1 }, { "SBC", &M::SBC, &M::IMM, 2, 2 }, /*E*/ { "NOP",  &M::NOP, &M::IMP, 2, 1 }, { "*SBC", &M::SBC, &M::IMM, 2, 2 }, { "CPX",  &M::CPX, &M::ABS, 4, 3 }, { "SBC", &M::SBC, &M::ABS, 4, 3 }, { "INC", &M::INC, &M::ABS, 6, 3 }, { "*ISB", &M::ISB, &M::ABS, 6, 3 }, /*E*/
    /*F*/ { "BEQ",  &M::BEQ, &M::REL, 2, 2 }, { "SBC", &M::SBC, &M::IIY, 5, 2 }, { "???", &M::XXX, &M::IMP, 2, 0 }, { "*ISB",  &M::ISB, &M::IIY, 8, 2 }, { "*NOP", &M::NO2, &M::ZPI, 3, 2 }, /*F*/ { "SBC", &M::SBC, &M::ZPX, 4, 2 }, { "INC", &M::INC, &M::ZPX, 6, 2 }, { "*ISB", &M::ISB, &M::ZPX, 6, 2 }, { "SED", &M::SED, &M::IMP, 2, 1 }, { "SBC", &M::SBC, &M::ABY, 4, 3 }, /*F*/ { "*NOP", &M::NOP, &M::IMP, 2, 1 }, { "*ISB", &M::ISB, &M::ABY, 7, 3 }, { "*NOP", &M::NOP, &M::ABX, 2, 3 }, { "SBC", &M::SBC, &M::ABX, 4, 3 }, { "INC", &M::INC, &M::ABX, 7, 3 }, { "*ISB", &M::ISB, &M::ABX, 7, 3 }, /*F*/
    //                    0                                  1                                  2                                    3                                   4                                        5                                  6                                   7                                  8                                  9                                         A                                   B                                   C                                  D                                  E                                   F
};

// clang-format on
