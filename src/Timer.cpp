#include "Timer.h"

#include <chrono>

void Timer::init(float ticksPerSecond_)
{
	ticksPerSecond = ticksPerSecond_;
	tickLength = 1000.0f / ticksPerSecond_;
	lastSystemClock = milliTime();
}

void Timer::update()
{
	uint64_t systemClock = milliTime();

	delta += float(systemClock - lastSystemClock) / tickLength;
	deltaTicks = int(delta);
	delta -= float(deltaTicks);
	lastSystemClock = systemClock;

	if (deltaTicks > MAX_DELTA_TICKS)
	{
		deltaTicks = MAX_DELTA_TICKS;
	}
}

void Timer::tick()
{
	ticks++;
}

uint64_t Timer::milliTime() 
{
	return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}
