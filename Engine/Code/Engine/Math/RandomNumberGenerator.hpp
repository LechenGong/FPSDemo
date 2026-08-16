#pragma once

#include <cstdlib>

class RandomNumberGenerator
{
public:
	int RollRandomIntLessThan( int maxNotInclusive );
	int RollRandomIntInRange( int minInclusive, int maxInclusive );
	float RollRandomFloatZeroToOne();
	float RollRandomFloatInRange( float minInlucisve, float maxInclusive );

private:
};

int RollRandomIntLessThan( int maxNotInclusive );
int RollRandomIntInRange( int minInclusive, int maxInclusive );
float RollRandomFloatZeroToOne();
float RollRandomFloatInRange( float minInlucisve, float maxInclusive );