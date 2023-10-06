#pragma once
#include "Noise.h"

#include <memory>

class CombinedNoise : public Noise {
public:
	CombinedNoise(std::shared_ptr<Noise> noise1, std::shared_ptr<Noise> noise2);

	float compute(float x, float y) override;
private:
	std::shared_ptr<Noise> noise1;
	std::shared_ptr<Noise> noise2;
};