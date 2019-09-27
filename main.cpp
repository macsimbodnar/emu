#include "mos6502.hpp"
#include "bus.hpp"
#include "log.hpp"
#include "console.hpp"
#include "common.hpp"
#include "cartridge.hpp"


int main(int argc, char *argv[]) {
    Console console;
    log_set_console(&console);

    // FILE *file;
    // long size;
    p_state_t state;

    if (argc < 2) {
        log_e("Provide the binary code!");
        return 1;
    }

    // file = fopen(argv[1], "rb");

    // if (file == nullptr) {
    //     log_e("Can not open the file " + std::string(argv[1]));
    //     return 1;
    // }

    // get the file size
    // fseek(file, 0, SEEK_END);
    // size = ftell(file);
    // fseek(file, 0, SEEK_SET);

    // std::string line_2 = "Total memory: " + std::to_string(mem_size) + " bytes";
    // std::string line_3 = "Binary size:     " + std::to_string(size) + " bytes";

    // console.set_header_line_2(line_2.c_str(), line_2.length());
    // console.set_header_line_3(line_3.c_str(), line_3.length());

    // // Load the code in memory
    // uint8_t *mem_ptr_off = mem_ptr + 0x4020;    // Set the offset to page 3

    // if (fread(mem_ptr_off, sizeof(uint8_t), size, file) != (size_t)size) {
    //     log_e("Failed read instructions from file");
    //     return 1;
    // }

    // fclose(file);

    // mem_ptr[0xFFFC] = 0x20;                     // Set the reset Vector
    // mem_ptr[0xFFFD] = 0x40;
    Cartridge test(argv[1]);

    if (!test.is_valid()) {
        log_e("The cartridge is not valid");
        return 1;
    }

    Bus b(&test);
    MOS6502 cpu(&b);

    uint8_t *mem_ptr = b.get_mem_ptr();
    size_t mem_size = b.get_mem_size();

    cpu.reset();

    cpu.set_PC(0xC000);

    state = cpu.get_status();


    while (console.frame(state, mem_ptr, mem_size)) {
        cpu.clock();
        state = cpu.get_status();

        uint8_t res1;
        uint8_t res2;

        b.access(0x0002, access_mode_t::READ, res1);
        b.access(0x0003, access_mode_t::READ, res2);

        printf("Result: 02 -> %d      03 -> %d\n\n\n", res1, res2);
    }

    return 0;
}

/**
 *
LDX #10
STX $0000
LDX #3
STX $0001
LDY $0000
LDA #0
CLC
loop
ADC $0001
DEY
BNE loop
STA $0002
NOP
NOP
NOP

**/