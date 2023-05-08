#pragma once
#include "Noise.h"

class CombinedNoise : public Noise {
public:
	CombinedNoise(Noise* noise1, Noise* noise2);
	~CombinedNoise() override;

	float compute(float x, float y) override;
private:
	Noise* noise1;
	Noise* noise2;
};