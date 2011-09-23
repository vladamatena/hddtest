/*******************************************************************************
*
*	HDDTest the graphical drive benchmarking tool.
*	Copyright (C) 2011  Vladimír Matěna <vlada.matena@gmail.com>
*
*	This program is free software: you can redistribute it and/or modify
*	it under the terms of the GNU General Public License as published by
*	the Free Software Foundation, either version 3 of the License, or
*	(at your option) any later version.
*
*	This program is distributed in the hope that it will be useful,
*	but WITHOUT ANY WARRANTY; without even the implied warranty of
*	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*	GNU General Public License for more details.
*
*	You should have received a copy of the GNU General Public License
*	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
********************************************************************************/

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
