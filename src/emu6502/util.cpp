#include <sstream>
#include <bitset>
#include <inttypes.h>
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
                "%.4X  %.2X       %4s                             A:%.2X X:%.2X Y:%.2X P:%.2X SP:%.2X PPU:XXX,XXX CYC:%d ms:%" PRId64,
                s.PC_executed, s.opcode, s.opcode_name.c_str(), s.A, s.X, s.Y, s.P, s.S, s.tot_cycles, s.time);
        break;

    case 2:
        sprintf(out,
                "%.4X  %.2X %.2X    %4s                             A:%.2X X:%.2X Y:%.2X P:%.2X SP:%.2X PPU:XXX,XXX CYC:%d ms:%" PRId64,
                s.PC_executed, s.opcode, s.arg1, s.opcode_name.c_str(), s.A, s.X, s.Y, s.P, s.S, s.tot_cycles, s.time);
        break;

    case 3:
        sprintf(out,
                "%.4X  %.2X %.2X %.2X %4s                             A:%.2X X:%.2X Y:%.2X P:%.2X SP:%.2X PPU:XXX,XXX CYC:%d ms:%" PRId64,
                s.PC_executed, s.opcode, s.arg1, s.arg2, s.opcode_name.c_str(), s.A, s.X, s.Y, s.P, s.S,
                s.tot_cycles, s.time);
        break;

    default:
        sprintf(out, "ERROR, unexpected opcode_size %d", s.opcode_size);
        break;
    }
}


int64_t time_diff(const timeval *t1, const timeval *t2) {
    // return ((t2->tv_sec + (t2->tv_usec / 1000000.0)) * 1000.0) -
    //        ((t1->tv_sec + (t1->tv_usec / 1000000.0)) * 1000.0);

    return (1000000 * t2->tv_sec + t2->tv_usec) - (1000000 * t1->tv_sec + t1->tv_usec);
}