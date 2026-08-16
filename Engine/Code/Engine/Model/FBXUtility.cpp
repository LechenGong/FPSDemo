#include "Engine/Model/FBXUtility.hpp"

float ConvertEModeToFrameRate( FbxTime::EMode mode )
{
	static const std::unordered_map<FbxTime::EMode, float> modeToFrameRate =
	{
		{FbxTime::eDefaultMode, 30.0f},
		{FbxTime::eFrames120, 120.0f},
		{FbxTime::eFrames100, 100.0f},
		{FbxTime::eFrames60, 60.0f},
		{FbxTime::eFrames50, 50.0f},
		{FbxTime::eFrames48, 48.0f},
		{FbxTime::eFrames30, 30.0f},
		{FbxTime::eFrames30Drop, 29.97f},
		{FbxTime::eNTSCDropFrame, 29.97f},
		{FbxTime::eNTSCFullFrame, 30.0f},
		{FbxTime::ePAL, 25.0f},
		{FbxTime::eFrames24, 24.0f},
		{FbxTime::eFrames1000, 1000.0f},
		{FbxTime::eFilmFullFrame, 24.0f},
		{FbxTime::eCustom, 30.0f},
		{FbxTime::eFrames96, 96.0f},
		{FbxTime::eFrames72, 72.0f},
		{FbxTime::eFrames59dot94, 59.94f},
		{FbxTime::eFrames119dot88, 119.88f}
	};

	auto iter = modeToFrameRate.find( mode );
	if (iter != modeToFrameRate.end())
	{
		return iter->second;
	}
	else
	{
		return 30.0f;
	}
}

FbxTime::EMode ConvertFrameRateToEMode( float frameRate )
{
	static const std::unordered_map<float, FbxTime::EMode> frameRateToMode =
	{
		{120.0f, FbxTime::eFrames120},
		{100.0f, FbxTime::eFrames100},
		{60.0f, FbxTime::eFrames60},
		{50.0f, FbxTime::eFrames50},
		{48.0f, FbxTime::eFrames48},
		{30.0f, FbxTime::eFrames30},
		{29.97f, FbxTime::eFrames30Drop},
		{29.97f, FbxTime::eNTSCDropFrame},
		{25.0f, FbxTime::ePAL},
		{24.0f, FbxTime::eFrames24},
		{1000.0f, FbxTime::eFrames1000},
		{24.0f, FbxTime::eFilmFullFrame},
		{96.0f, FbxTime::eFrames96},
		{72.0f, FbxTime::eFrames72},
		{59.94f, FbxTime::eFrames59dot94},
		{119.88f, FbxTime::eFrames119dot88},
		{30.0f, FbxTime::eNTSCFullFrame} 
	};

	for (const auto& pair : frameRateToMode)
	{
		if (std::fabs( pair.first - frameRate ) < 1e-2) 
		{
			return pair.second;
		}
	}

	return FbxTime::eDefaultMode;
}

Mat44 ConvertFbxAMatrixToMat44( FbxAMatrix const& fbxAMatrix )
{
	Mat44 mat44;
	for (int col = 0; col < 4; col++)
	{
		for (int row = 0; row < 4; row++)
		{
			mat44.m_values[row * 4 + col] = float( fbxAMatrix.Get( row, col ) );
		}
	}
	return mat44;
}

FbxAMatrix ConvertMat44ToFbxAMatrix( Mat44 const& mat44 )
{
	FbxAMatrix fbxAMatrix;
	for (int col = 0; col < 4; col++)
	{
		for (int row = 0; row < 4; row++)
		{
			fbxAMatrix.mData[col][row] = double( mat44.m_values[col * 4 + row] );
		}
	}
	return fbxAMatrix;
}
