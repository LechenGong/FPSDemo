#pragma once

#include <vector>

#include "Engine/Core/Vertex_PCUTBN.hpp"

class Vertex_Anim
{
public:
	Vertex_Anim() {};
// 	std::vector<unsigned int> m_jointIndexes;
// 	std::vector<float> m_jointWeights;
	unsigned int m_jointIndexes[4] = {0, 0, 0, 0};
	float m_jointWeights[4] = {0.f, 0.f, 0.f, 0.f};
};
