/**
* \class RandomGenerator
*
* Implementation of predictable random number generator.
* Provides two methods for querying random numbers (32 and 64 bit).
* Uses random with states internaly.
* Random number stability is guaranted only on the same machine with thw same SW versions/.
*
* \author Vladimír Matěna
*
* Contact vlada.matena@gmail.com
*
*/


#ifndef RANDOMGENERATOR_H
#define RANDOMGENERATOR_H

#include <QtGlobal>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <iostream>

using namespace std;

class RandomGenerator
{
public:
	RandomGenerator();	// initalize generator

	qint32 Get32();	// get predictable 32bit integer
	qint64 Get64();	// get predictable 64bit integer

private:
	static const int statelen = 10;

	struct random_data data;
	char state[statelen];
};

#endif // RANDOMGENERATOR_H
