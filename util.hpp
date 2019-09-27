#pragma once

#include <string>

std::string uint16_to_hex(const uint16_t i, bool prefix = false);
std::string uint8_to_bin(const uint8_t i);
std::string uint16_to_bin(const uint16_t i);
void build_log_str(std::string &out, const uint16_t PC);
