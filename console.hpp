#pragma once
#include <stdint.h>
#include "common.hpp"

class Console {
  private:
    static const unsigned int WIDTH = 150;
    static const unsigned int HEIGHT = 50;

    static const unsigned int HEADER_HEIGHT = 4;
    static const unsigned int MEM_WIDTH = 16;

    static const unsigned int STATUS_X = 60;

    char display[HEIGHT][WIDTH];

    unsigned int print_mem_page;
    p_state_t current_state;
    
    void draw_memory(uint8_t *mem, const size_t mem_size);
    void draw_status();
    void draw_logs();
    void show();
    bool get_input();

  public:
    Console();

    bool frame(p_state_t &state, uint8_t *mem, const size_t mem_size);

    void set_header_line_1(const char * str, size_t size);
    void set_header_line_2(const char * str, size_t size);
    void set_header_line_3(const char * str, size_t size);
};