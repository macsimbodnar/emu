/**
 * min address: 0x0000
 * max address: 0xFFFF
 */

#pragma once
#include <stdint.h>
#include <array>


class Bus {
  private:
    static const uint16_t MIN_ADDRESS = 0x0000;
    static const uint16_t MAX_ADDRESS = 0xFFFF;

    std::array<uint8_t, 64 * 1024> RAM;

  public:
    Bus();

    enum access_t {
        READ = 0,
        WRITE
    };

    void access(const uint16_t address, const access_t read_write, uint8_t &data);

    size_t gem_mem_size();
    uint8_t * get_mem_ptr();
};
