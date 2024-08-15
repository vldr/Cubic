#pragma once
#include <cstdint>
#include <stdint.h>

class Random {
public:
  Random() = default;
  Random(uint64_t seed);

  void init(uint64_t seed);
  int64_t integerRange(int64_t min, int64_t max);
  uint64_t integer();
  double uniform();
  double uniformRange(double min, double max);

private:
  uint64_t seed;
  uint64_t state;
};