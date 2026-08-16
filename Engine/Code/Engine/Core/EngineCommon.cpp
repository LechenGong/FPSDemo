#include "Engine/Core/EngineCommon.hpp"

NamedStrings g_gameConfig; 

bool DoFloatRangeOverlap( FloatRange A, FloatRange B )
{
	float a = (A.x > B.x) ? A.x : B.x;
	float b = (A.y < B.y) ? A.y : B.y;
	return a < b;
}

