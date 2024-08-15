#pragma once
#include "OctaveNoise.h"

class CombinedNoise {
public:
  CombinedNoise(OctaveNoise noise1, OctaveNoise noise2);

  float compute(float x, float y);
private:
  OctaveNoise noise1;
  OctaveNoise noise2;
};