#pragma once

#define FBXSDK_SHARED
#include "fbxsdk.h"

#include <unordered_map>

#include "Engine/Math/Mat44.hpp"

float ConvertEModeToFrameRate( FbxTime::EMode mode );

FbxTime::EMode ConvertFrameRateToEMode( float frameRate );

Mat44 ConvertFbxAMatrixToMat44( FbxAMatrix const& fbxAMatrix );

FbxAMatrix ConvertMat44ToFbxAMatrix( Mat44 const& mat44 );