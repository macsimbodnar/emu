#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include <fstream>

#include <bus.hpp>
#include <mos6502.hpp>
#include <cartridge.hpp>
#include <util.hpp>


#define TEST_CARTRIDGE          "nestest.nes"
#define LOG_FILE                "nestest.log"
#define TEST_START_LOCATION     0xC000
// C69A  8D 06 40  STA $4006 = FF                  A:FF X:FF Y:15 P:A5 SP:FB PPU:140,233 CYC:26538
#define LOG_INST_LEN            19
#define LOG_REG_OFFSET          48
#define LOG_REG_LEN             25


TEST_CASE("Test") {
    // Open the log file used to check the correct cpu behavior
    std::ifstream log_file(LOG_FILE);
    REQUIRE(log_file);
    char line[150];
    char state_log[150];
    p_state_t state;
    p_state_t previous_state;
    int cmp_res;
    // Init the CPU, bus and cartridge
    Cartridge nestest(TEST_CARTRIDGE);
    REQUIRE(nestest.is_valid());

    Bus b(&nestest);
    MOS6502 cpu(&b);
    cpu.reset();
    cpu.set_PC(TEST_START_LOCATION);
    previous_state = cpu.get_status();
    unsigned int iteration = 0;

    while (log_file) {
        // Read log file line by line
        log_file.getline(line, 255);

        if (log_file) {
            iteration++;
            // Exec next instruction
            cpu.clock();
            p_state_t curr_state = cpu.get_status();

            // SOME STATE MAGIC TO MAKE MATCH THE LOG FILE
            state = curr_state;

            state.P = previous_state.P;
            state.S = previous_state.S;
            state.A = previous_state.A;
            state.X = previous_state.X;
            state.Y = previous_state.Y;

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
        }

    }

    log_file.close();
}
