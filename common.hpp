#pragma once
#include <stdint.h>
#include <string>


struct p_state_t {
    uint8_t A;
    uint8_t X;
    uint8_t Y;
    uint8_t S;  // Stack pointer
    uint8_t P;  // Processor stats 
    uint16_t PC;

    std::string opcode_name;
    uint8_t opcode;
    unsigned int opcode_size;

    uint8_t data_bus;

    uint16_t address;
    uint16_t relative_adderess;

    uint16_t tmp_buff;

    unsigned int cycles_count;
    unsigned int cycles_needed;

    uint16_t PC_executed;
    uint8_t arg1;
    uint8_t arg2;

    uint32_t tot_cycles;
};


enum class access_mode_t {
    READ = 0,
    WRITE,
    READ_ONLY
};