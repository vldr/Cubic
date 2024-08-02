#pragma once
#include "Noise.h"
#include "Random.h"

#include <memory>

class OctaveNoise : public Noise {
public:
  OctaveNoise(Random& random, int octaveCount);

  float compute(float x, float y) override;
private:
  int octaveCount;
  std::unique_ptr<std::unique_ptr<Noise>[]> noises;
};
