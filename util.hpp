#pragma once

#include <string>
#include <array>
#include <cstring>
#include <sys/time.h>
#include <stdint.h>
#include "common.hpp"

// Functions
std::string uint16_to_hex(const uint16_t i, bool prefix = false);
std::string uint8_to_bin(const uint8_t i);
std::string uint16_to_bin(const uint16_t i);
void build_log_str(char *out, const p_state_t &s);
int64_t time_diff(const timeval *t1, const timeval *t2);


// Classes
template <typename T, uint32_t S>
class Queue {
  private:
    uint32_t m_capacity;
    uint32_t m_size;
    uint32_t m_front;
    uint32_t m_rear;

    std::array<T, S> m_memory;

  public:
    Queue() : m_capacity(S), m_size(0), m_front(0), m_rear(S - 1) {}

    inline bool is_full() const {
        return (m_size == m_capacity);
    }

    inline bool is_empty() const {
        return (m_size == 0);
    }


    inline bool insert_in_front(const T &elem) {
        if (is_full()) {
            return false;
        }

        m_front = (m_front + m_capacity - 1) % m_capacity;
        m_memory[m_front] = elem;
        m_size++;
        return true;
    }

    inline bool enqueue(const T &elem) {
        if (is_full()) {
            return false;
        }

        m_rear = (m_rear + 1) % m_capacity;
        m_memory[m_rear] = elem;
        m_size++;
        return true;
    }

    inline bool dequeue(T &elem_out) {
        if (is_empty()) {
            return false;
        }

        elem_out = m_memory[m_front];
        m_front = (m_front + 1) % m_capacity;
        m_size--;
        return true;
    }

    inline bool front(T &elem_out) const {
        if (is_empty()) {
            return false;
        }

        elem_out = m_memory[m_front];
        return true;
    }

    inline bool rear(T &elem_out) const {
        if (is_empty()) {
            return false;
        }

        elem_out = m_memory[m_rear];
        return true;
    }

    inline void clear() {
        m_size = 0;
        m_front = 0;
        m_rear = S - 1;
    }
};