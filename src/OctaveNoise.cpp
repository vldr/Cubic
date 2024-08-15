#include "OctaveNoise.h"
#include "PerlinNoise.h"

OctaveNoise::OctaveNoise(Random& random, int octaveCount) : octaveCount(octaveCount)
{
  for (int i = 0; i < octaveCount; i++) 
  {
    noises.emplace_back(random);
  }
}

float OctaveNoise::compute(float x, float y) 
{
  float a = 0.0;
  float b = 1.0;

  for (int i = 0; i < octaveCount; i++) 
  {
    a += noises[i].compute(x / b, y / b) * b;
    b *= 2;
  }

  return a;
}
