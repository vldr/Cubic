#include "Random.h"

#include <glm/glm.hpp>

void Random::init(uint64_t seed_)
{
	seed = seed_;
	state = seed_;
}

uint64_t Random::integer() 
{
	state ^= (state >> 12);
	state ^= (state << 25);
	state ^= (state >> 27);
	return state * 2685821657736338717ULL;
}

int64_t Random::integerRange(int64_t min, int64_t max) 
{
	return min + integer() % (max + 1 - min);
}

double Random::uniform() 
{
	return (double)integer() / (double)UINT64_MAX;
}

double Random::uniformRange(double min, double max) 
{
	return min + uniform() * (max - min);
}