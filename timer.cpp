/**
* \author Vladimír Matěna vlada.matena@gmail.com
*/

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

hddtime Timer::GetFinalOffset()
{
	return (end.tv_sec * s + end.tv_usec * us) - (start.tv_sec * s + start.tv_usec * us);
}

hddtime Timer::GetCurrentOffset()
{
	timeval now;
	gettimeofday(&now, 0);

	return (now.tv_sec * s + now.tv_usec * us) - (start.tv_sec * s + start.tv_usec * us);
}
