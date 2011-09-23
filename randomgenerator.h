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

#ifndef RANDOMGENERATOR_H
#define RANDOMGENERATOR_H

#include <QtGlobal>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <iostream>

using namespace std;

/// Implements deterministic random genrator
/** Implementation of predictable random number generator.
Provides two methods for querying random numbers (32 and 64 bit).
Uses random with states internaly.
Random number stability is guaranted only on the same machine with thw same SW versions **/
class RandomGenerator
{
public:
	RandomGenerator();	/// Initalize generator with stable seed

	qint32 Get32();	/// Gets predictible 32bit integer
	qint64 Get64();	/// Gets predictible 64bit integer

private:
	static const int statelen = 10;

	struct random_data data;
	char state[statelen];
};

#endif // RANDOMGENERATOR_H
