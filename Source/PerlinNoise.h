#pragma once
#include "Noise.h"

class Random;

class PerlinNoise : public Noise {
public:
	PerlinNoise(Random& random);
	~PerlinNoise() override;

	float compute(float x, float y) override;

private:
	float f(float x);
	float lerp(float t, float a, float b);
	float grad(int i, float x, float y, float z);
private:
	int hash[512];
};
