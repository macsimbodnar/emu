#include <iostream>

#include "log.hpp"

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