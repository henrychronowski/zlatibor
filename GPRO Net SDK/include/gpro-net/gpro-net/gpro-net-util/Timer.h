#ifndef _TIMER_H
#define _TIMER_H

#include <chrono>
#include <ctime>
#include <cmath>

typedef std::chrono::time_point<std::chrono::steady_clock> time_point;
typedef std::chrono::milliseconds milliseconds;

class Timer
{
public:
	void Start();
	double Stop();

private:
	time_point mStartTime;
	//std::chrono::time_point<std::chrono::steady_clock> mEndTime;
	bool isStarted = false;
};


#endif // !_TIMER_H
