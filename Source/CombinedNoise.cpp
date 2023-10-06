#include "CombinedNoise.h"

CombinedNoise::CombinedNoise(std::shared_ptr<Noise> noise1, std::shared_ptr<Noise> noise2) : noise1(noise1), noise2(noise2)
{
}

float CombinedNoise::compute(float x, float y)
{
	return noise1->compute(x + noise2->compute(x, y), y);
}
