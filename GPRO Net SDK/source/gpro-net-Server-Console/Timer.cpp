#include "Timer.h"

void Timer::Start()
{
	mStartTime = clock::now();
	isStarted = true;
}

double Timer::Stop()
{
	time_point end = clock::now();
	double duration = 0.0;
	if (isStarted)
	{
		duration = std::chrono::duration_cast<milliseconds>(end - mStartTime).count();
		isStarted = false;
	}
	
	return duration;
}