#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>

#include <fstream>

#include "mos6502.hpp"
#include "common.hpp"
#include "util.hpp"


#define TEST_CARTRIDGE          "resources/nestest.nes"
#define LOG_FILE                "resources/nestest.log"
#define TEST_START_LOCATION     0xC000
// C69A  8D 06 40  STA $4006 = FF                  A:FF X:FF Y:15 P:A5 SP:FB PPU:140,233 CYC:26538
#define LOG_INST_LEN            19
#define LOG_REG_OFFSET          48
#define LOG_REG_LEN             25
#define LOG_CYC_OFFSET          87

#define NES_PRG_BANK_SIZE       16384
#define NES_CHR_BANK_SIZE       8192
#define NES_RAM                 2048

// iNES Format Header
struct ines_header_t {
    char name[4];
    uint8_t prg_rom_chunks;
    uint8_t chr_rom_chunks;
    uint8_t mapper1;
    uint8_t mapper2;
    uint8_t prg_ram_size;
    uint8_t tv_system1;
    uint8_t tv_system2;
    char unused[5];
};

enum class mirror_t {
    HORIZONTAL,
    VERTICAL,
    ONESCREEN_LO,
    ONESCREEN_HI,
};


struct NES_cartridge_t {
    std::vector<uint8_t> prg_memory;
    std::vector<uint8_t> chr_memory;

    uint8_t prg_banks;
    uint8_t chr_banks;

    uint8_t mapper_id;
    mirror_t mirror;

    uint8_t RAM[NES_RAM];
};


static void log_clb(const std::string &log);
static void mem_callback(void *usr_data,
                         const uint16_t address,
                         const access_mode_t read_write,
                         uint8_t &data);

static bool load_NES_cartridge(const char *file, NES_cartridge_t &cartridge_out);


TEST_CASE("Test") {
    char state_log[150];
    p_state_t state;
    p_state_t previous_state;
    int cmp_res = 0;
    unsigned int iteration = 0;
    NES_cartridge_t cartridge;

    // Open the log file used to check the correct cpu behavior
    std::ifstream log_file(LOG_FILE);
    REQUIRE(log_file);
    char line[150];

    // Load the NES test cartridge. Execute it at address 0xC000 and compare the log with the log_file
    bool cartridge_load_res = load_NES_cartridge(TEST_CARTRIDGE, cartridge);
    REQUIRE(cartridge_load_res);

    // Initialize the cpu and set the log callback
    MOS6502 cpu(mem_callback, (void *)&cartridge);
    cpu.set_log_callback(log_clb);

    // Reset the cpu before use and set the Program Counter to specific mem addres in order to perfrom all tests
    cpu.reset();
    cpu.set_PC(TEST_START_LOCATION);
    previous_state = cpu.get_status();

    while (log_file) {
        // Read log file line by line
        log_file.getline(line, 255);

        if (log_file) {
            iteration++;

            // Exec next instruction
            while (!cpu.clock()) {};

            p_state_t curr_state = cpu.get_status();

            // SOME STATE MAGIC TO MAKE MATCH THE LOG FILE
            state = curr_state;

            state.P = previous_state.P;

            state.S = previous_state.S;

            state.A = previous_state.A;

            state.X = previous_state.X;

            state.Y = previous_state.Y;

            state.tot_cycles = previous_state.tot_cycles;

            previous_state = curr_state;

            build_log_str(state_log, state);

            // Compare current instruction
            cmp_res = memcmp(line, state_log, LOG_INST_LEN);

            printf("%s\n", state_log);

            if (cmp_res != 0) {
                printf("INSTRUCTION Missmatch on iteration %d\n%s\n", iteration, line);
            }

            REQUIRE_EQ(cmp_res, 0);

            // Compare registers
            cmp_res = memcmp(line + LOG_REG_OFFSET, state_log + LOG_REG_OFFSET, LOG_REG_LEN);

            if (cmp_res != 0) {
                printf("REGISTER Missmatch on iteration %d\n%s\n", iteration, line);
            }

            // Compare registers
            size_t len = strlen(line);
            cmp_res = memcmp(line + LOG_CYC_OFFSET, state_log + LOG_CYC_OFFSET, len - LOG_CYC_OFFSET);

            if (cmp_res != 0) {
                printf("CYCLES COUNT Missmatch on iteration %d\n%s\n", iteration, line);
            }

            REQUIRE_EQ(cmp_res, 0);
        }

    }

    log_file.close();
}


static void log_clb(const std::string &log) {
    printf("%s\n", log.c_str());
}


static void mem_callback(void *usr_data,
                         const uint16_t address, const access_mode_t read_write, uint8_t &data) {

    NES_cartridge_t *cartridge = (NES_cartridge_t *) usr_data;

    if (cartridge == nullptr) {
        log_clb("The cartridge is nullptr inside the mem_access_callback");
        return;
    }

    // Access the cartridge cpu memory

    // First get the address from the mapper
    // if PRG ROM is 16KB
    //     CPU Address Bus          PRG ROM
    //     0x8000 -> 0xBFFF: Map    0x0000 -> 0x3FFF
    //     0xC000 -> 0xFFFF: Mirror 0x0000 -> 0x3FFF
    // if PRG ROM is 32KB
    //     CPU Address Bus          PRG ROM
    //     0x8000 -> 0xFFFF: Map    0x0000 -> 0x7FFF
    if (address >= 0x8000 && address <= 0xFFFF) {
        uint16_t mapped_addr = address & (cartridge->prg_banks > 1 ? 0x7FFF : 0x3FFF);

        // READ
        if (read_write == access_mode_t::READ) {
            data = cartridge->prg_memory[mapped_addr];
        }
        // WRITE
        else if (read_write == access_mode_t::WRITE) {
            cartridge->prg_memory[mapped_addr] = data;
        }
        // Error case
        else {
            log_clb("Unexpected mem access type in address handled by the cartridge");
        }

        return;
    }

    // If here means that the mapper not handle this address so we use the RAM
    if (address >= 0x0000 && address <= 0x1FFF) {
        // READ
        if (read_write == access_mode_t::READ) {
            data = cartridge->RAM[address];
        }
        // WRITE
        else if (read_write == access_mode_t::WRITE) {
            cartridge->RAM[address] = data;
        }
        // Error case
        else {
            log_clb("Unexpected mem access type in RAM address space");
        }

        return;
    }

    // If here means the address is have to be handled by the PPU. Is ignored for this test
    if (address >= 0x2000 && address <= 0x3FFF) {
        log_clb("PPU mem access. Access type: " +
                ((read_write == access_mode_t::READ) ? std::string("READ") : std::string("WRITE")) +
                std::string(" at address ") + std::to_string(address) +
                std::string(" with data value: ") + std::to_string(data));

        return;
    }

    // Unexpected address handling
    log_clb("Unexpected address: " + std::to_string(address));
}


static bool load_NES_cartridge(const char *file, NES_cartridge_t &cartridge_out) {
    FILE *fp = fopen(file, "rb");
    long size;
    ines_header_t header;

    if (fp == nullptr) {
        log_clb("Can not open the file " + std::string(file));
        fclose(fp);
        return false;
    }

    // get the file size
    fseek(fp, 0, SEEK_END);
    size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    if ((size_t)size <= sizeof(ines_header_t))  {
        log_clb("Invalid cartridge file " + std::string(file));
        fclose(fp);
        return false;
    }

    // Read the header
    size_t b_readen = fread((char *)&header, sizeof(char), sizeof(ines_header_t), fp);

    if (b_readen != sizeof(ines_header_t)) {
        log_clb("Failed read the cartridge from file: " + std::string(file));
        fclose(fp);
        return false;
    }

    // If a "trainer" exists we just need to read past
    // it before we get to the good stuff
    if (header.mapper1 & 0x04) {
        fseek(fp, 0, 512);
    }

    // Determine Mapper ID
    cartridge_out.mapper_id = ((header.mapper2 >> 4) << 4) | (header.mapper1 >> 4);
    cartridge_out.mirror = (header.mapper1 & 0x01) ? mirror_t::VERTICAL : mirror_t::HORIZONTAL;

    if (cartridge_out.mapper_id != 0) {
        log_clb("unexpected mapper in the cartridge: " + std::string(file));
        fclose(fp);
        return false;
    }

    cartridge_out.prg_banks = header.prg_rom_chunks;
    cartridge_out.prg_memory.resize(cartridge_out.prg_banks * NES_PRG_BANK_SIZE);

    size_t read_res = fread((uint8_t *) cartridge_out.prg_memory.data(),
                            sizeof(uint8_t),
                            cartridge_out.prg_memory.size(),
                            fp);

    if (read_res != cartridge_out.prg_memory.size()) {
        log_clb("Failed read the cartridge program memory from file: " + std::string(file));
        fclose(fp);
        return false;
    }

    cartridge_out.chr_banks = header.chr_rom_chunks;
    cartridge_out.chr_memory.resize(cartridge_out.chr_banks * NES_CHR_BANK_SIZE);

    read_res = fread((uint8_t *) cartridge_out.chr_memory.data(),
                     sizeof(uint8_t),
                     cartridge_out.chr_memory.size(),
                     fp);

    if (read_res != cartridge_out.chr_memory.size()) {
        log_clb("Failed read the cartridge character memory from file: " + std::string(file));
        fclose(fp);
        return false;
    }

    fclose(fp);

    return true;
}