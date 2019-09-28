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

TEST_CASE("Test") {
    // Open the log file used to check the correct cpu behavior
    std::ifstream log_file(LOG_FILE);
    REQUIRE(log_file);
    char line[150];
    char state_log[150];
    p_state_t state;
    int cmp_res;
    // Init the CPU, bus and cartridge
    Cartridge nestest(TEST_CARTRIDGE);
    REQUIRE(nestest.is_valid());

    Bus b(&nestest);
    MOS6502 cpu(&b);
    cpu.set_PC(TEST_START_LOCATION);
    unsigned int iteration = 0;
    while (log_file) {
        // Read log file line by line
        log_file.getline(line, 255);

        if (log_file) {
            iteration++;
            // Exec next instruction
            cpu.clock();
            state = cpu.get_status();
            build_log_str(state_log, state);

            // Compare current instruction
            cmp_res = memcmp(line, state_log, 19);

            if (cmp_res != 0 ) {
                printf("Missmatch on iteration %d\nexpected: %s\ncurrent:  %s\n", iteration, line, state_log);
            }

            REQUIRE_EQ(cmp_res, 0);
        }

    }

    log_file.close();
}
