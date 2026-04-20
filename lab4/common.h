#pragma once

#include <cstdint>

inline constexpr uint32_t MSG_SIZE = 20;

struct RingHeader {
    uint32_t capacity;
    uint32_t head;
    uint32_t tail;
    uint32_t count;
};

inline constexpr uint32_t HEADER_SIZE = sizeof(RingHeader);
