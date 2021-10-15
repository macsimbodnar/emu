#pragma once
#include <stdint.h>
#include <string>

enum class access_mode_t { // Access mode type
  READ = 0,                // Read from memory
  WRITE,                   // Write to memory
  READ_ONLY
};

/**
 * Callback used by the MOS6502 Class to read / write memory
 *
 * usr_data     User data will be passed here
 * address      Address of memory to access in range from 0x0000 to 0xFFFF
 * read_write   Memory access mode. If READ data stored at 'address' will be
 *              copied in 'data', if WRITE 'data' will be copied at 'address'
 * data         Data used to store/read. If 'read_write' is READ 'data' is
 *              output parameter, if WRITE 'data' is input parameter
 * */
typedef void (*mem_access_callback)(void *usr_data, const uint16_t address,
                                    const access_mode_t read_write,
                                    uint8_t &data);

// Data structure returned by get_status()
// and contain the current status of the MOS6502
struct p_state_t {
  uint8_t A;   // Register A
  uint8_t X;   // Index    X
  uint8_t Y;   // Index    Y
  uint8_t S;   // Stack pointer
  uint8_t P;   // Processor stats
  uint16_t PC; // Program counter

  // Mnemonic name of the last executed instruction
  std::string opcode_name;
  uint8_t opcode; // Last executed operational code

  // The size of byte of the current opcode. Can be 1, 2 or 3
  unsigned int opcode_size;

  uint8_t data_bus; // last data passed through the data bus

  uint16_t address;           // Last abbsolute address used
  uint16_t relative_adderess; // Last relative address
                              // Last data stored in temporary buffer used for
                              // inner logic
  uint16_t tmp_buff;

  // Cycles left for current opcode. NOTE(max): currently allways 0
  unsigned int cycles_count;
  // Cycles need fo last opcode to complete. NOTE(max): currently not accurate
  unsigned int cycles_needed;

  uint16_t PC_executed; // Address of the last opcode executed
                        // Argument 1 of the last opcode executed. Is valid only
                        // if opcode_size > 1
  uint8_t arg1;
  // Argument 2 of the last opcode executed. Is valid only if opcode_size > 2
  uint8_t arg2;

  // Total cycles executed by the cpu. NOTE(max): currently not used
  uint32_t tot_cycles;
  int64_t time; // Time that this cycle take to execute in ms
};

// This is the callback that the cpu use to log. If set the CPU will log, if not
// the log will be just skipped
typedef void (*log_callback)(const std::string &log);