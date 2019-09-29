/**
 * min address: 0x0000
 * max address: 0xFFFF
 */

#pragma once
#include <stdint.h>
#include <array>
#include "cartridge.hpp"
#include "common.hpp"


class Bus {
  private:
    // 2KB of RAM
    std::array<uint8_t, 2048> RAM;
    Cartridge *cartridge;

  public:
    Bus(Cartridge *cartridge);

    static void access(void * self, const uint16_t address, const access_mode_t read_write, uint8_t &data);

    size_t get_mem_size();
    uint8_t *get_mem_ptr();
};
