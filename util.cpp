#include <sstream>
#include <bitset>
#include "util.hpp"
#include "common.hpp"


std::string uint16_to_hex(const uint16_t i, bool prefix) {
    std::stringstream stream;
    stream << std::hex << i;

    return std::string((prefix ? "0x" : "") + stream.str());
}


std::string uint8_to_bin(const uint8_t i) {
    return std::bitset<8>(i).to_string();
}


std::string uint16_to_bin(const uint16_t i) {
    return std::bitset<16>(i).to_string();
}


void build_log_str(char *out, const p_state_t &s) {
    switch (s.opcode_size) {
    case 1:
        sprintf(out,
                "%.4X %.2X        %s                              A:%2X X:%2X Y:%2X P:%2X SP:%2X PPU:XXX,XXX CYC:%d",
                s.PC_executed, s.opcode, s.opcode_name.c_str(), s.A, s.X, s.Y, s.P, s.S, s.tot_cycles);
        break;

    case 2:
        sprintf(out,
                "%.4X %.2X %.2X     %s                              A:%2X X:%2X Y:%2X P:%2X SP:%2X PPU:XXX,XXX CYC:%d",
                s.PC_executed, s.opcode, s.arg1, s.opcode_name.c_str(), s.A, s.X, s.Y, s.P, s.S, s.tot_cycles);
        break;

    case 3:
        sprintf(out,
                "%.4X %.2X %.2X %.2X  %s                              A:%2X X:%2X Y:%2X P:%2X SP:%2X PPU:XXX,XXX CYC:%d",
                s.PC_executed, s.opcode, s.arg1, s.arg2, s.opcode_name.c_str(), s.A, s.X, s.Y, s.P, s.S,
                s.tot_cycles);
        break;

    default:
        sprintf(out, "ERROR, unexpected opcode_size %d", s.opcode_size);
        break;
    }


}