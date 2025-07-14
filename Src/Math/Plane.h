#pragma once

#include "glm/glm.hpp"

struct SPlane
{
	glm::vec3 Normal { 0.f, 1.f, 0.f };
	glm::vec3 Point { 0.0f, 0.0f, 0.0f };
	float GetSignedDistance(glm::vec3 point);
	bool IsSphereOnPlane(glm::vec3 sphereLoc, float radius); // IsSphereOnHalfspace would be more accurate but w/e...
};

