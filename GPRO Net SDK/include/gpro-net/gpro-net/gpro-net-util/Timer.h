#ifndef _TIMER_H
#define _TIMER_H

#include <chrono>
#include <ctime>
#include <cmath>

typedef std::chrono::time_point<std::chrono::steady_clock> time_point;
typedef std::chrono::milliseconds milliseconds;
typedef std::chrono::seconds seconds;

class Timer
{
public:
	void Start();
	double Stop();
	double Lap();

private:
	time_point mStartTime;
	bool isStarted = false;
};


#endif // !_TIMER_H
