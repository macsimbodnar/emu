#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

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

#define TIMING_TEST_BIN         "resources/6502timing/timingtest.bin"
#define TIMING_TEST_LOG_FILE    "resources/6502timing/timingtest.log"
#define TIMING_TEST_MEM_LOC     0x1000
#define TIMING_TEST_PC_END      0x1269
// On visual6502 it takes 1141 cycles, PC should be in 1269 hex
#define TIMING_TEST_TOT_CYCLES  1141

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


TEST_CASE("NES Test") {
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

            REQUIRE_EQ(cmp_res, 0);

            // Compare cycles
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


TEST_CASE("Cycles Timing Test") {
    uint8_t mem[64 * 1024];
    // Initialize the cpu and set the log callback
    MOS6502 cpu(
    [](void *usr_data, const uint16_t address, const access_mode_t read_write, uint8_t &data) -> void {
        uint8_t *mem = (uint8_t *) usr_data;

        switch (read_write) {
        case access_mode_t::READ:
            data = mem[address];
            break;

        case access_mode_t::WRITE:
            mem[address] = data;
            break;

        default:
            log_clb("Unexpected mem access type");
            break;
        }
    },
    (void *)mem);

    cpu.set_log_callback(log_clb);

    // Reset the cpu before use and set the Program Counter to specific mem addres in order to perfrom all tests
    cpu.reset();
    cpu.set_PC(TIMING_TEST_MEM_LOC);

    // The code starts from 0x1000
    mem[0xFFFC] = (uint8_t)TIMING_TEST_MEM_LOC & 0x00FF;                       // Set the reset Vector ll
    mem[0xFFFD] = (uint8_t)(TIMING_TEST_MEM_LOC >> 8) & 0x00FF;                 // Set the reset Vector hh

    // NOTE(max):   Using the srec from this discussion http://forum.6502.org/viewtopic.php?f=8&t=3340
    //              Loaded at address $1000

    // Read the file
    FILE *file = fopen(TIMING_TEST_BIN, "rb");
    REQUIRE_NE(file, nullptr);

    // get the file size
    fseek(file, 0, SEEK_END);
    size_t size = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Load the code in mem
    size_t read = fread(mem + TIMING_TEST_MEM_LOC, sizeof(uint8_t), size, file);
    REQUIRE_EQ(read, size);

    fclose(file);

    // Open the log file used to check the correct cpu behavior
    std::ifstream log_file(TIMING_TEST_LOG_FILE);
    REQUIRE(log_file);
    char line[150];

    for (int i = 0; i < 5; i++) {
        // Consume first 5 lines
        log_file.getline(line, 255);
    }

    cpu.cycles = 2;
    p_state_t curr_state;
    p_state_t old_state = cpu.get_status();
    char state_log[150];
    int iteration = 0;
    int cmp_res;
    uint64_t last_expected_cyc = 784803;
    uint64_t last_current_cyc = 0;
    char *endptr;

    while (log_file) {
        // Read log file line by line
        log_file.getline(line, 255);

        if (log_file) {
            iteration++;

            while (!cpu.clock()) {};

            curr_state = cpu.get_status();

            build_log_str(state_log, curr_state);

            // Compare PC
            cmp_res = memcmp(line + 11, state_log, 4);

            printf("%s\n", state_log);

            if (cmp_res != 0) {
                printf("PC Missmatch on iteration %d\n%s\n", iteration, line);
            }

            REQUIRE_EQ(cmp_res, 0);

            // Compare MNEMONIC INSTRUCTION
            cmp_res = memcmp(line + 18, state_log + 16, 3);

            if (cmp_res != 0) {
                printf("INSTRUCTION Missmatch on iteration %d\n%s\n", iteration, line);
            }

            REQUIRE_EQ(cmp_res, 0);

            // Compare cycles number
            uint64_t tmp = strtoul(line + 3, &endptr, 10);
            uint64_t expected_cyc = tmp - last_expected_cyc;
            uint64_t current_cyc = old_state.tot_cycles - last_current_cyc;

            if (expected_cyc != current_cyc) {
                printf("CYCLE Missmatch on iteration %d\nExpect: %" PRIu64 " Current: %" PRIu64 "\n%s\n",
                       iteration, expected_cyc, current_cyc, line);
            }

            REQUIRE_EQ(expected_cyc, current_cyc);

            last_current_cyc = old_state.tot_cycles;
            last_expected_cyc = tmp;
            old_state = curr_state;
        }
    }

    log_file.close();

    // Compare the cycles number
    curr_state = cpu.get_status();
    printf("%d\n", curr_state.tot_cycles);
}


TEST_CASE("Queue Test") {
    Queue<int, 10> q;

    REQUIRE_FALSE(q.is_full());
    REQUIRE(q.is_empty());

    for (int i = 0; i < 10; i++) {
        REQUIRE_FALSE(q.is_full());
        REQUIRE(q.enqueue(i));
        REQUIRE_FALSE(q.is_empty());
    }

    REQUIRE(q.is_full());
    int tmp;
    REQUIRE(q.front(tmp));
    REQUIRE_EQ(tmp, 0);

    REQUIRE(q.rear(tmp));
    REQUIRE_EQ(tmp, 9);

    for (int i = 0; i < 10; i++) {
        int elem;
        REQUIRE(q.dequeue(elem));
        REQUIRE_EQ(i, elem);
        REQUIRE_FALSE(q.is_full());
    }

    for (int i = 0; i < 10; i++) {
        REQUIRE_FALSE(q.is_full());
        REQUIRE(q.enqueue(i));
        REQUIRE_FALSE(q.is_empty());
    }

    REQUIRE(q.is_full());
    q.clear();
    REQUIRE_FALSE(q.is_full());
    REQUIRE(q.is_empty());

    for (int i = 0; i < 9; i++) {
        REQUIRE_FALSE(q.is_full());
        REQUIRE(q.enqueue(i));
        REQUIRE_FALSE(q.is_empty());
    }

    REQUIRE_FALSE(q.is_full());

    REQUIRE(q.insert_in_front(42));

    REQUIRE(q.is_full());

    REQUIRE(q.front(tmp));
    REQUIRE_EQ(42, tmp);

    REQUIRE(q.rear(tmp));
    REQUIRE_EQ(8, tmp);

    REQUIRE(q.dequeue(tmp));
    REQUIRE_EQ(42, tmp);

    for (int i = 0; i < 9; i++) {
        int elem;
        REQUIRE(q.dequeue(elem));
        REQUIRE_EQ(i, elem);
        REQUIRE_FALSE(q.is_full());
    }

    REQUIRE(q.is_empty());

    for (int i = 0; i < 5; i++) {
        REQUIRE_FALSE(q.is_full());
        REQUIRE(q.enqueue(i));
        REQUIRE_FALSE(q.is_empty());
    }

    REQUIRE(q.insert_in_front(42));

    REQUIRE_FALSE(q.is_full());

    REQUIRE(q.front(tmp));
    REQUIRE_EQ(42, tmp);

    REQUIRE(q.rear(tmp));
    REQUIRE_EQ(4, tmp);

    REQUIRE(q.dequeue(tmp));
    REQUIRE_EQ(42, tmp);

    for (int i = 0; i < 4; i++) {
        int elem;
        REQUIRE(q.dequeue(elem));
        REQUIRE_EQ(i, elem);
        REQUIRE_FALSE(q.is_full());
    }
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