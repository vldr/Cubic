#include "Timer.h"

#include <chrono>

void Timer::init(Game* game, float ticksPerSecond)
{
	this->ticks = 0;
	this->elapsedDelta = 0.0;
	this->adjustment = 1.0;
	this->ticksPerSecond = ticksPerSecond;
	this->lastSystemClock = milliTime();
	this->lastHRClock = nanoTime() / 1000000;
}

void Timer::update()
{
	uint64_t currentTime = milliTime();
	uint64_t timeSinceLastSystemClock = currentTime - lastSystemClock;
	uint64_t currentTimeInMilliseconds = nanoTime() / 1000000;

	double speedAdjustment;
	if (timeSinceLastSystemClock > 1000)
	{
		uint64_t timeSinceLastHRClock = currentTimeInMilliseconds - lastHRClock;
		speedAdjustment = (double)timeSinceLastSystemClock / timeSinceLastHRClock;

		adjustment += (float)((speedAdjustment - adjustment) * 0.2);
		lastSystemClock = currentTime;
		lastHRClock = currentTimeInMilliseconds;
	}

	if (timeSinceLastSystemClock < 0)
	{
		lastSystemClock = currentTime;
		lastHRClock = currentTimeInMilliseconds;
	}

	double currentTimeInSeconds = (double)currentTimeInMilliseconds / 1000.0;
	speedAdjustment = (currentTimeInSeconds - lastHR) * adjustment;
	lastHR = (float)currentTimeInSeconds;

	if (speedAdjustment < 0.0) { speedAdjustment = 0.0; }
	if (speedAdjustment > 1.0) { speedAdjustment = 1.0; }

	elapsedDelta += (float)(speedAdjustment * ticksPerSecond);
	elapsedTicks = (int)elapsedDelta;

	if (elapsedTicks > 100) { elapsedTicks = 100; }

	elapsedDelta -= elapsedTicks;
	delta = elapsedDelta;
}

void Timer::tick()
{
	ticks++;
}

uint64_t Timer::nanoTime() 
{
	std::chrono::system_clock::duration chronoDuration = std::chrono::system_clock::now().time_since_epoch();
	std::chrono::seconds chronoSeconds = std::chrono::duration_cast<std::chrono::seconds>(chronoDuration);

	auto seconds = chronoSeconds.count();
	auto nanoSeconds = std::chrono::duration_cast<std::chrono::microseconds>(chronoDuration - chronoSeconds).count();

	return 1000 * ((uint64_t)(seconds % 10000) * 1000000 + (uint64_t)nanoSeconds);
}

uint64_t Timer::milliTime() 
{
	return nanoTime() / 1000000;
}
