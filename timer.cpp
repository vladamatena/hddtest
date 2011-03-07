#include "timer.h"

Timer::Timer():
	start(timeval()), end(timeval()) {}

void Timer::MarkStart()
{
	gettimeofday(&start, 0);
}

void Timer::MarkEnd()
{
	gettimeofday(&end, 0);
}

hddtime Timer::GetOffset()
{
	return (end.tv_sec * s + end.tv_usec * us) - (start.tv_sec * s + start.tv_usec * us);
}
