#include <stdio.h>
#include <cstring>
#include "console.hpp"
#include "util.hpp"

#define EMPTY ' '


Console::Console() : print_mem_page(0) {
    for (unsigned int j = 0; j < HEIGHT; j++) {
        for (unsigned int i = 0; i < WIDTH; i++) {
            display[j][i] = EMPTY;

            if (j == 0 || i == WIDTH - 1 || j == HEIGHT - 1) {
                display[j][i] = '*';
            }
        }

    }

    memcpy(&(display[3][25]), "MEMORY", sizeof("MEMORY") - 1);
    memcpy(&(display[3][70]), "STATE", sizeof("STATE") - 1);
}


bool Console::frame(p_state_t &state, uint8_t *p_mem, const size_t size) {
    current_state = state;
    mem = p_mem;
    mem_size = size;
    draw_memory();
    draw_status();
    draw_logs();

    show();

    return get_input();
}


void Console::draw_memory() {

    size_t from = print_mem_page * 255;
    size_t to = from + 255;

    size_t line = HEADER_HEIGHT;

    buff = "PAGE: ";
    if (print_mem_page < 10) {
        buff += "00";
    } else if (print_mem_page < 100) {
        buff += "0";
    }

    buff += std::to_string(print_mem_page);
    memcpy(&(display[line - 1][32]), &(buff[0]), buff.length());

    for (size_t i = from; i < to;) {

        size_t col = 0;
        col = sprintf(&(display[line][col]), "0x%04X  ", ((unsigned int)i) & 0x0000FFFF);
        col--;

        for (size_t j = 0; j < MEM_WIDTH; j++, i++) {

            if (i == current_state.cur_abb_add) {
                sprintf(&(display[line][col]), "[%02X", mem[i]);
            } else {
                sprintf(&(display[line][col]), " %02X", mem[i]);
            }

            col += 3;
        }

        display[line][col] = EMPTY;
        line++;
    }


    return;
}


void Console::draw_status() {
    unsigned int line = HEADER_HEIGHT;
    unsigned int col = STATUS_X;

    // FLAGS
    col += 5;
    buff = "N O - B D I Z C";
    memcpy(&(display[line][col]), &(buff[0]), buff.length());

    line++;
    buff = std::string(current_state.N ? "1 " : "0 ") +
           (current_state.O ? "1 " : "0 ") +
           "- " +
           (current_state.B ? "1 " : "0 ") +
           (current_state.D ? "1 " : "0 ") +
           (current_state.I ? "1 " : "0 ") +
           (current_state.Z ? "1 " : "0 ") +
           (current_state.C ? "1 " : "0");

    memcpy(&(display[line][col]), &(buff[0]), buff.length());

    // A register
    line += 2;
    col = STATUS_X;
    buff = "A: [" + uint8_to_bin(current_state.A) + "]         XXXX";
    sprintf(&(buff[22]), "0x%02X", current_state.A);
    memcpy(&(display[line][col]), &(buff[0]), buff.length());

    // X register
    line++;
    buff = "X: [" + uint8_to_bin(current_state.X) + "]         XXXX";
    sprintf(&(buff[22]), "0x%02X", current_state.X);
    memcpy(&(display[line][col]), &(buff[0]), buff.length());

    // Y register
    line++;
    buff = "Y: [" + uint8_to_bin(current_state.Y) + "]         XXXX";
    sprintf(&(buff[22]), "0x%02X", current_state.Y);
    memcpy(&(display[line][col]), &(buff[0]), buff.length());

    // PC
    line ++;
    col--;
    buff = "PC: [" + uint16_to_bin(current_state.PC) + "] XXXXXX";
    sprintf(&(buff[23]), "0x%04X", current_state.PC);
    memcpy(&(display[line][col]), &(buff[0]), buff.length());

    // Opcode
    line += 2;
    col = STATUS_X - 1;
    buff = "OP: [" + uint8_to_bin(current_state.opcode) + "]    " +
           current_state.opcode_name + "  XXXX";
    sprintf(&(buff[23]), "0x%02X", current_state.opcode);
    memcpy(&(display[line][col]), &(buff[0]), buff.length());

    // Fetched
    line++;
    buff = "FE: [" + uint8_to_bin(current_state.fetched) + "]         XXXX";
    sprintf(&(buff[23]), "0x%02X", current_state.fetched);
    memcpy(&(display[line][col]), &(buff[0]), buff.length());

    // Current abbsolute address
    line += 2;
    col = STATUS_X - 2;
    buff = "ADR: [" + uint16_to_bin(current_state.cur_abb_add) + "] XXXXXX";
    sprintf(&(buff[24]), "0x%04X", current_state.cur_abb_add);
    memcpy(&(display[line][col]), &(buff[0]), buff.length());

    // Current relative address
    line ++;
    buff = "REL: [" + uint16_to_bin(current_state.cur_rel_add) + "] XXXXXX";
    sprintf(&(buff[24]), "0x%04X", current_state.cur_rel_add);
    memcpy(&(display[line][col]), &(buff[0]), buff.length());

    // Tmp buffer
    line ++;
    buff = "TMP: [" + uint16_to_bin(current_state.tmp_buff) + "] XXXXXX";
    sprintf(&(buff[24]), "0x%04X", current_state.tmp_buff);
    memcpy(&(display[line][col]), &(buff[0]), buff.length());

    // Cycle counters
    line += 2;
    col = STATUS_X;
    buff = "CYCL N: ";
    if (current_state.cycles_count < 10) {
        buff += "0";
    }
    buff += std::to_string(current_state.cycles_count) + "   CYCL NEED: ";
    if (current_state.cycles_needed < 10) {
        buff += "0";
    }
    buff += std::to_string(current_state.cycles_needed);
    memcpy(&(display[line][col]), &(buff[0]), buff.length());
}


void Console::draw_logs() {
    return;
}


void Console::show() {
    printf("\x1b[H\x1b[J"); // Erase the screen

    for (unsigned int j = 0; j < HEIGHT; j++) {
        printf("%.*s\n", WIDTH, &(display[j][0]));
    }
}


bool Console::get_input() {
    char in;

    while (true) {
        scanf(" %c", &in);

        switch (in) {
        case 'c':
        case 'C':
            return true;

        case 'n':
        case 'N':
            if (print_mem_page < 0xFFFF) {
                print_mem_page++;
                draw_memory();
                show();
            }

            break;

        case 'b':
        case 'B':
            if (print_mem_page > 0) {
                print_mem_page--;
                draw_memory();
                show();
            }

            break;

        case 'q':
        case 'Q':
            return false;

        default:
            break;
        }
    }

    return false;
}


void Console::set_header_line_2(const char *str, size_t size) {
    memcpy(&(display[1][0]), str, size);
}

void Console::set_header_line_3(const char *str, size_t size) {
    memcpy(&(display[2][0]), str, size);
}