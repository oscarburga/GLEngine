#include "Plane.h"

float SPlane::GetSignedDistance(glm::vec3 point) const
{
	return glm::dot(Normal, point - Point);
}

bool SPlane::IsSphereOnPlane(glm::vec3 sphereLoc, float radius) const
{
	return -radius < GetSignedDistance(sphereLoc);
}
