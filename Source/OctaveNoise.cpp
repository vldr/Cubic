#include "OctaveNoise.h"
#include "PerlinNoise.h"

OctaveNoise::OctaveNoise(Random* random, int octaveCount) : octaveCount(octaveCount) 
{
	noises = new Noise*[octaveCount];

	for (int i = 0; i < octaveCount; i++) {
		noises[i] = new PerlinNoise(random);
	}
}

float OctaveNoise::compute(float x, float y) 
{
	float a = 0.0;
	float b = 1.0;

	for (int i = 0; i < octaveCount; i++) 
	{
		a += noises[i]->compute(x / b, y / b) * b;
		b *= 2;
	}

	return a;
}

OctaveNoise::~OctaveNoise() 
{
	for (int i = 0; i < octaveCount; i++) 
	{
		delete noises[i];
	}

	delete[] noises;
}