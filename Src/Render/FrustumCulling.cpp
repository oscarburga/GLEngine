#include "FrustumCulling.h"

#include <algorithm>

float SPlane::GetSignedDistance(glm::vec3 point)
{
	return glm::dot(Normal, point - Point);
}

bool SPlane::IsSphereOnPlane(vec3 sphereLoc, float radius)
{
	return -radius < GetSignedDistance(sphereLoc);
}

bool SFrustum::IsSphereInFrustum(const SBounds& bounds, const mat4& transform)
{
	const vec3 scale(glm::length(glm::vec3(transform[0])), glm::length(glm::vec3(transform[1])), glm::length(glm::vec3(transform[2])));
	// const float maxScale = std::max({ glm::abs(scale.x), glm::abs(scale.y), glm::abs(scale.z) });
	const float maxScale = std::max({ scale.x, scale.y, scale.z });
	const vec3 worldPos = transform * glm::vec4(bounds.Origin, 1.f);
	const float worldRadius = maxScale * bounds.Radius; // *0.5f;
	const bool bResult = std::all_of(Planes.begin(), Planes.end(), [&](SPlane& p)
	{
		return p.IsSphereOnPlane(worldPos, worldRadius);
	});
	return bResult;
}
