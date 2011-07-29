/**
* \author Vladimír Matěna vlada.matena@gmail.com
*/

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
