#pragma once

#include "glm/glm.hpp"

struct SBounds
{
	float Radius; 
	glm::vec3 Origin;
	glm::vec3 Extent; // Extent is the offset from the middle to opposite bounding box corners
};
