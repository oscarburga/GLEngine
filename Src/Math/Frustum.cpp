#include "Frustum.h"

#include <algorithm>
#include "Bounds.h"

bool SFrustum::IsSphereInFrustum(const SBounds& bounds, const glm::mat4& transform) const
{
	const glm::vec3 scale(glm::length(glm::vec3(transform[0])), glm::length(glm::vec3(transform[1])), glm::length(glm::vec3(transform[2])));
	const glm::vec3 worldPos = transform * glm::vec4(bounds.Origin, 1.f);
	// const float maxScale = std::max({ glm::abs(scale.x), glm::abs(scale.y), glm::abs(scale.z) });
	const float maxScale = std::max({ scale.x, scale.y, scale.z });
	const float worldRadius = maxScale * bounds.Radius; // *0.5f;
	const bool bResult = std::all_of(Planes.begin(), Planes.end(), [&](const SPlane& p)
	{
		return p.IsSphereOnPlane(worldPos, worldRadius);
	});
	return bResult;
}
