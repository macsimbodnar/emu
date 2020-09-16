#pragma once

#include <mos6502.hpp>

class CPU {
  private:
    MOS6502 cpu;

    static void mem_callback(void *usr_data, const uint16_t address, const access_mode_t read_write, uint8_t &data);

  public:
    CPU();
};