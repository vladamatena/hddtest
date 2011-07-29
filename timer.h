/**
* \author Vladimír Matěna vlada.matena@gmail.com
*/

#ifndef TIMER_H
#define TIMER_H

#include <sys/time.h>

#include "definitions.h"

using namespace HDDTest;

/// Class implementing software stopwatch
/** The Timer class works like stopwatch.
  The start and stop position can be marked. **/
class Timer
{
public:
	Timer();	/// The Timer constuctor

	void MarkStart();	/// Marks start time
	void MarkEnd();		/// Marks end time

	/** Gets time difference between start and stop time
	  @return the difference is returned as hddtime
	  @see definitions.h for time constants **/
	hddtime GetFinalOffset();

	/** Sets time difference between start and current time
	  @return the difference is returned as hddtime
	  @see definitions.h for time constants **/
	hddtime GetCurrentOffset();

private:
	timeval start;
	timeval end;
};

#endif // TIMER_H
