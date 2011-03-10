#ifndef TIMER_H
#define TIMER_H

#include <sys/time.h>

#include "definitions.h"

using namespace HDDTest;

class Timer
{
public:
	Timer();

	void MarkStart();
	void MarkEnd();
	hddtime GetFinalOffset();
	hddtime GetCurrentOffset();

private:
	timeval start;
	timeval end;
};

#endif // TIMER_H