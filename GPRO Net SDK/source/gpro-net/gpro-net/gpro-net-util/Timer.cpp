#include "gpro-net/gpro-net/gpro-net-util/Timer.h"

void Timer::Start()
{
	mStartTime = std::chrono::steady_clock::now();
	isStarted = true;
}

double Timer::Stop()
{
	time_point end = std::chrono::steady_clock::now();
	double duration = 0.0;
	if (isStarted)
	{
		duration = (double)std::chrono::duration_cast<milliseconds>(end - mStartTime).count();
		isStarted = false;
	}
	
	return duration;
}

double Timer::Lap()
{
	time_point end = std::chrono::steady_clock::now();
	double duration = 0.0;
	if (isStarted)
	{
		duration = (double)std::chrono::duration_cast<milliseconds>(end - mStartTime).count();
	}

	return duration;
}