#pragma once
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/Mat44.hpp"

struct FootIKConfig
{
	std::string thighJointName;
	std::string kneeJointName;
	std::string footJointName;
};

struct TwoBoneIKInput
{
	Vec3 rootPos;
	Vec3 midPos;
	Vec3 endPos;
	Vec3 targetPos;
	Vec3 poleVector;

	Mat44 rootTransform;
	Mat44 midTransform;
	Mat44 endTransform;
};

struct TwoBoneIKResult
{
	Vec3 solvedMidPos;
	Vec3 solvedEndPos;

	Mat44 rootWorldTransform;
	Mat44 midWorldTransform;
	Mat44 endWorldTransform;
};

class IKSolver
{
public:
	// Solve a simple thigh-knee-foot chain in world space.
	static bool SolveTwoBoneIK( TwoBoneIKInput const& input, TwoBoneIKResult& output );
};