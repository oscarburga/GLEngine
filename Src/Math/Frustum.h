#pragma once

#include <array>
#include "Plane.h"

struct SBounds;
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

