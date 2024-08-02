#include "PerlinNoise.h"
#include "Random.h"

#include <glm/glm.hpp>

PerlinNoise::PerlinNoise(Random& random) 
{
  for (int i = 0; i < 256; i++) { hash[i] = i; }
  for (int i = 0; i < 256; i++) {
    int r1 = (int)random.integerRange(0, 256 - i - 1) + i;
    int r2 = hash[i];
    hash[i] = hash[r1];
    hash[r1] = r2;
    hash[i + 256] = hash[i];
  }
}

PerlinNoise::~PerlinNoise()
{
}

float PerlinNoise::f(float x) 
{
  return x * x * x * (x * (x * 6.0f - 15.0f) + 10.0f);
}

float PerlinNoise::lerp(float t, float a, float b) 
{
  return a + t * (b - a);
}

float PerlinNoise::grad(int i, float x, float y, float z) 
{
  i &= 15;
  float a = i < 8 ? x : y;
  float b = i < 4 ? y : (i != 12 && i != 14 ? z : x);
  return ((i & 1) == 0 ? a : -a) + ((i & 2) == 0 ? b : -b);
}

float PerlinNoise::compute(float x, float y) 
{
  float vx = x, vy = y, vz = 0.0f;
  int ix = ((int)vx) & 255, iy = ((int)vy) & 255, iz = ((int)vz) & 255;
  vx -= glm::floor(vx);
  vy -= glm::floor(vy);
  vz -= glm::floor(vz);
  float xd = f(vx), yd = f(vy), zd = f(vz);
  int aaa, aba, aab, abb, baa, bba, bab, bbb;
  aaa = hash[hash[hash[ix] + iy] + iz];
  aba = hash[hash[hash[ix] + iy + 1] + iz];
  aab = hash[hash[hash[ix] + iy] + iz + 1];
  abb = hash[hash[hash[ix] + iy + 1] + iz + 1];
  baa = hash[hash[hash[ix + 1] + iy] + iz];
  bba = hash[hash[hash[ix + 1] + iy + 1] + iz];
  bab = hash[hash[hash[ix + 1] + iy] + iz + 1];
  bbb = hash[hash[hash[ix + 1] + iy + 1] + iz + 1];
  float l1 = lerp(xd, grad(aaa, vx, vy, vz), grad(baa, vx - 1.0f, vy, vz));
  float l2 = lerp(xd, grad(aba, vx, vy - 1.0f, vz), grad(bba, vx - 1.0f, vy - 1.0f, vz));
  float l3 = lerp(xd, grad(aab, vx, vy, vz - 1.0f), grad(bab, vx - 1.0f, vy, vz - 1.0f));
  float l4 = lerp(xd, grad(abb, vx, vy - 1.0f, vz - 1.0f), grad(bbb, vx - 1.0f, vy - 1.0f, vz - 1.0f));
  float l = lerp(zd, lerp(yd, l1, l2), lerp(yd, l3, l4));

  return l;
}
