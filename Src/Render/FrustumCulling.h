#pragma once

#include <array>
#include "Math/EngineMath.h"

struct SBounds
{
	float Radius; 
	glm::vec3 Origin;
	glm::vec3 Extent; // Extent is the offset from the middle to opposite bounding box corners
};

struct SPlane
{
	glm::vec3 Normal { 0.f, 1.f, 0.f };
	glm::vec3 Point { 0.0f, 0.0f, 0.0f };
	float GetSignedDistance(glm::vec3 point);
	bool IsSphereOnPlane(glm::vec3 sphereLoc, float radius); // IsSphereOnHalfspace would be more accurate but w/e...
};

/*
* For left/right/bottom/top, the name corresponds to the actual plane in the frustrum.
* Left plane is the left plane, with a normal pointing towards the right.
* Bottom plane is the bottom plane, with a normal pointing upwards, etc.
*/
struct SFrustum
{
	union
	{
		struct
		{
			SPlane Near;
			SPlane Far;
			SPlane Left;
			SPlane Right;
			SPlane Bottom;
			SPlane Top;
		};
		std::array<SPlane, 6> Planes {};
	};
	bool IsSphereInFrustum(const SBounds& bounds, const glm::mat4& transform);
};

namespace EFrustumCorners
{
	enum
	{
		NearBotLeft = 0,
		NearBotRight = 1,
		NearTopRight = 2,
		NearTopLeft = 3,
		FarBotLeft = 4,
		FarBotRight = 5,
		FarTopRight = 6,
		FarTopLeft = 7,
	};
}

