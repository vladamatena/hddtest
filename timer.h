#ifndef TIMER_H
#define TIMER_H

#include <sys/time.h>

#include "definitions.h"

using namespace HDDTest;

class Timer
{
public:
    Timer();
	inline void MarkStart();
	inline void MarkEnd();
	hddtime GetOffset();

private:
	timeval start;
	timeval end;
};

#endif // TIMER_H
