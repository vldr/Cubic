#include "CombinedNoise.h"

CombinedNoise::CombinedNoise(std::unique_ptr<Noise> noise1, std::unique_ptr<Noise> noise2) 
	: noise1(std::move(noise1)), noise2(std::move(noise2))
{
}

float CombinedNoise::compute(float x, float y)
{
	return noise1->compute(x + noise2->compute(x, y), y);
}
