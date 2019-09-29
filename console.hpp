#pragma once
#include <stdint.h>
#include <array>
#include "common.hpp"

class Console {
  private:
    static const unsigned int LOG_LINES = 6;
    static const unsigned int CONTENT_HEIGHT = 22;

    static const unsigned int WIDTH = 95;
    static const unsigned int HEIGHT = LOG_LINES + CONTENT_HEIGHT + 1;

    static const unsigned int HEADER_HEIGHT = 4;
    static const unsigned int MEM_WIDTH = 16;

    static const unsigned int STATUS_X = 60;

    char display[HEIGHT][WIDTH];

    unsigned int print_mem_page;
    p_state_t current_state;
    uint8_t *mem;
    size_t mem_size;
    std::string buff;

    std::array<std::string, LOG_LINES> logs;
    unsigned int log_head = 0;
    unsigned int log_count = 0;

    void draw_memory();
    void draw_status();
    void draw_exec_log();
    void draw_logs();
    void show();
    bool get_input();

    void push_log(const std::string &str);

    void set_header_line_2(const char *str, size_t size);
    void set_header_line_3(const char *str, size_t size);


  public:
    Console();
    int run(int argc, char **argv);
};