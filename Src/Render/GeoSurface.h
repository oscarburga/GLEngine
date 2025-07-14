#pragma once

#include <memory>

#include "Math/Bounds.h"

struct SPbrMaterial;

struct SGeoSurface 
{
	uint32_t StartIndex = 0;
	uint32_t Count = 0;
	SBounds Bounds {};
	std::shared_ptr<SPbrMaterial> Material {};
};
