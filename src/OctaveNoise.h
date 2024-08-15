#pragma once
#include "PerlinNoise.h"
#include "Random.h"

#include <vector>

class OctaveNoise {
public:
  OctaveNoise(Random& random, int octaveCount);

  float compute(float x, float y);
private:
  int octaveCount;
  std::vector<PerlinNoise> noises;
};
