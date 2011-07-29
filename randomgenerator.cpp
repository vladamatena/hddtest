/**
* \author Vladimír Matěna vlada.matena@gmail.com
*/

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
