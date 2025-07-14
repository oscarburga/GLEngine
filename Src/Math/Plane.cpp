#include "Plane.h"

float SPlane::GetSignedDistance(glm::vec3 point)
{
	return glm::dot(Normal, point - Point);
}

bool SPlane::IsSphereOnPlane(glm::vec3 sphereLoc, float radius)
{
	return -radius < GetSignedDistance(sphereLoc);
}
