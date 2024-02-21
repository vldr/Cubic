#pragma once
#include <cstdint>

class Timer
{
public:
	void init(float ticksPerSecond_);
	void update();
	void tick();

	uint64_t milliTime();

	int ticks;
	float ticksPerSecond;
	float tickLength;

	int deltaTicks;
	float delta;
private:
	uint64_t lastSystemClock;
};

