#pragma once

#include <array>
#include <vector>

typedef std::array<uint8_t, 32> h256_t;

struct Range {
  Range(uint64_t start = 0, uint64_t count = 0) : start(start), count(count) {}
  uint64_t start;
  uint64_t count;
};
