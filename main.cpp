#include "mos6502.hpp"
#include "bus.hpp"
#include "log.hpp"

int main(int argc, char *argv[]) {

    Bus b;
    MOS6502 cpu(&b);
    FILE *ptr;
    long size;

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

    log_i("Total memory: " + std::to_string(b.get_max_addr() + 1) + " bytes");
    log_i("Binary size: " + std::to_string(size) + " bytes");

    // Load the code in memory
    if (fread(b.get_mem_ptr(), sizeof(uint8_t), size, ptr) != (size_t)size) {
        log_e("Failed read instructions from file");
        return 1;
    }

    return 0;
}