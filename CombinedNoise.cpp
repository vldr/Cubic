#include "CombinedNoise.h"

CombinedNoise::CombinedNoise(Noise* noise1, Noise* noise2) : noise1(noise1), noise2(noise2)
{
}

CombinedNoise::~CombinedNoise()
{
}

float CombinedNoise::compute(float x, float y)
{
	return noise1->compute(x + noise2->compute(x, y), y);
}
