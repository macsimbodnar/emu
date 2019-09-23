#include "mos6502.hpp"
#include "bus.hpp"
#include "log.hpp"
#include "console.hpp"
#include "common.hpp"


int main(int argc, char *argv[]) {
    Console console;
    Bus b;
    MOS6502 cpu(&b);
    FILE *ptr;
    long size;
    p_state_t state = cpu.get_status();
    uint8_t *mem_ptr = b.get_mem_ptr();
    size_t mem_size = b.gem_mem_size();

    if (argc < 2) {
        log_e("Provide the binary code!");
        return 1;
    }

    ptr = fopen(argv[1], "rb");

    if (ptr == nullptr) {
        log_e("Can not open the file " + std::string(argv[1]));
        return 1;
    }

    // get the file size
    fseek(ptr, 0, SEEK_END);
    size = ftell(ptr);
    fseek(ptr, 0, SEEK_SET);

    std::string line_2 = "Total memory: " + std::to_string(mem_size) + " bytes";
    std::string line_3 = "Binary size: " + std::to_string(size) + " bytes";
    
    console.set_header_line_2(line_2.c_str(), line_2.length());
    console.set_header_line_3(line_3.c_str(), line_3.length());

    // Load the code in memory
    if (fread(mem_ptr, sizeof(uint8_t), size, ptr) != (size_t)size) {
        log_e("Failed read instructions from file");
        return 1;
    }

    while (console.frame(state, mem_ptr, mem_size)) {
        cpu.clock();
        state = cpu.get_status();
    }

    return 0;
}