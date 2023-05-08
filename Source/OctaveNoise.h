#pragma once
#include "Noise.h"

class Random;

class OctaveNoise : public Noise {
public:
	OctaveNoise(Random* random, int octaveCount);
	~OctaveNoise() override;

	float compute(float x, float y) override;
private:
	int octaveCount;
	Noise** noises;
};
