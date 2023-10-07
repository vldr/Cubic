#pragma once
#include "Noise.h"

#include <memory>

class CombinedNoise : public Noise {
public:
	CombinedNoise(std::unique_ptr<Noise> noise1, std::unique_ptr<Noise> noise2);

	float compute(float x, float y) override;
private:
	std::unique_ptr<Noise> noise1;
	std::unique_ptr<Noise> noise2;
};