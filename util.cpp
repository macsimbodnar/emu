#include <sstream>
#include <bitset>
#include "util.hpp"


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