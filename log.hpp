#pragma once
#include <string>
#include "console.hpp"

#define log_i log_e

void log_e(const std::string &msg);
void log_set_console(Console *console);