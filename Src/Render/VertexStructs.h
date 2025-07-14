#pragma once

#include "glm/glm.hpp"

struct SVertex
{
	glm::vec3 Position = {};
	float uv_x = 0.0f;
	glm::vec3 Normal = {};
	float uv_y = 0.0f;
	glm::vec4 Color = {};
	glm::vec4 Tangent = {}; // xyz normalized, W is the sign (+-1) indicating tangent handedness
};

struct SVertexSkinData 
{
	glm::uvec4 Joints {};
	glm::vec4 Weights {};
};
