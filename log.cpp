#include <iostream>

#include "log.hpp"

// TODO(max): remove console dep
static Console *console = nullptr;


void log_set_console(Console *con) {
    console = con;
}


void log_e(const std::string &msg) {
    if (console) {
        console->push_log(msg);
    } else {
        std::cout << msg << std::endl;
    }
}