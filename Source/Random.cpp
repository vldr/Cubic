#include "Random.h"
#include <glm/glm.hpp>

void Random::init(uint64_t seed)
{
	this->seed = seed;
	this->state = seed;
	this->lastNormal = INFINITY;
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

double Random::normal(double stddev) 
{
	if (lastNormal != INFINITY) {
		double r = lastNormal;
		lastNormal = INFINITY;
		return r * stddev;
	}

	const auto PI = 3.14159265358979;
	double r = glm::sqrt(-2.0 * glm::log(1.0 - uniform()));
	double phi = 2.0 * PI * (1.0 - uniform());

	lastNormal = r * glm::cos(phi);
	return r * glm::sin(phi) * stddev;
}
