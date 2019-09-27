#pragma once

#include <string>
#include "common.hpp"

std::string uint16_to_hex(const uint16_t i, bool prefix = false);
std::string uint8_to_bin(const uint8_t i);
std::string uint16_to_bin(const uint16_t i);
void build_log_str(char *out, const p_state_t &s);
