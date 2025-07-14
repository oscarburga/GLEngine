#pragma once

#include "glm/glm.hpp"

struct SSceneData
{
	constexpr static int MAX_CASCADES = 16;
	glm::vec4 CameraPos {};
	glm::vec4 SunlightDirection {}; // 4th component for sun power
	glm::vec4 SunlightColor {};
	glm::mat4 View {};
	glm::mat4 Proj {};
	glm::mat4 ViewProj {};
	// TODO: change this to not use vec4 per element. STD140 is stupid and rounds every element of a float[] array to be the same size as a vec4.
	glm::vec4 CascadeDistances[MAX_CASCADES] = {}; 
	glm::mat4 LightSpaceTransforms[MAX_CASCADES] {};
};
