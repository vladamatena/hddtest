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

#include "randomgenerator.h"

RandomGenerator::RandomGenerator()
{
	memset(&data, 0, sizeof(random_data));
	initstate_r(1, state, statelen, &data);
	time_t t;
	time(&t);
	srandom(t);
}

qint32 RandomGenerator::Get32()
{
	int32_t result;
	random_r(&data, &result);

	return result;
}

qint64 RandomGenerator::Get64()
{
	return ((qint64)( Get32()) << 32) | Get32();
}
