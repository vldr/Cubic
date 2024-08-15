#pragma once

class Random;

class PerlinNoise {
public:
  PerlinNoise(Random& random);

  float compute(float x, float y);
private:
  float f(float x);
  float lerp(float t, float a, float b);
  float grad(int i, float x, float y, float z);
private:
  int hash[512];
};
