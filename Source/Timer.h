#pragma once
#include <cstdint>

class Game;

class Timer
{
public:
	uint64_t nanoTime();
	uint64_t milliTime();
	void init(Game* game, float ticksPerSecond);
	void update();
	void tick();

	int ticks;
	int elapsedTicks;

	float delta;
	float ticksPerSecond;
private:
	float lastHR;
	float elapsedDelta;
	uint64_t lastSystemClock;
	uint64_t lastHRClock;
	float adjustment;
};

