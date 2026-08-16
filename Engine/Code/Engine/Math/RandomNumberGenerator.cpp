#include "Engine/Math/RandomNumberGenerator.hpp"

int RandomNumberGenerator::RollRandomIntLessThan( int maxNotInclusive )
{
	return rand() % maxNotInclusive;
}

int RandomNumberGenerator::RollRandomIntInRange( int minInclusive, int maxInclusive )
{
	if (minInclusive == maxInclusive)
		return maxInclusive;

	return rand() % (maxInclusive - minInclusive + 1) + minInclusive;
}

float RandomNumberGenerator::RollRandomFloatZeroToOne()
{
	return (float)rand() / (float)(RAND_MAX);
}

float RandomNumberGenerator::RollRandomFloatInRange( float minInlucisve, float maxInclusive )
{
	if (minInlucisve == maxInclusive)
		return maxInclusive;

	return RollRandomFloatZeroToOne() * (maxInclusive - minInlucisve) + minInlucisve;
}

int RollRandomIntLessThan( int maxNotInclusive )
{
	return rand() % maxNotInclusive;
}

int RollRandomIntInRange( int minInclusive, int maxInclusive )
{
	if (minInclusive == maxInclusive)
		return maxInclusive;

	return rand() % (maxInclusive - minInclusive + 1) + minInclusive;
}

float RollRandomFloatZeroToOne()
{
	return (float)rand() / (float)(RAND_MAX);
}

float RollRandomFloatInRange( float minInlucisve, float maxInclusive )
{
	if (minInlucisve == maxInclusive)
		return maxInclusive;

	return RollRandomFloatZeroToOne() * (maxInclusive - minInlucisve) + minInlucisve;
}
