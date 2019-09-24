#pragma once
#include <stdint.h>
#include <string>


struct p_state_t {
    uint8_t A;
    uint8_t X;
    uint8_t Y;
    uint8_t S;
    uint16_t PC;

    bool N;
    bool O;
    bool U;
    bool B;
    bool D;
    bool I;
    bool Z;
    bool C;

    std::string opcode_name;
    uint8_t opcode;

    uint8_t data_bus;

    uint16_t address;
    uint16_t relative_adderess;

    uint16_t tmp_buff;

    unsigned int cycles_count;
    unsigned int cycles_needed;
};