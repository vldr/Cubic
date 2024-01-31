#pragma once
#include <stdint.h>

class Random {
public:
	void init(uint64_t seed);
	uint64_t integer();
	int64_t integerRange(int64_t min, int64_t max);
	double uniform();
	double uniformRange(double min, double max);
	
private:
	uint64_t seed;
	uint64_t state;
	double lastNormal;
};