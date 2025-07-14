#pragma once

#include <string>
#include <vector>
#include <array>
#include <unordered_map>
#include <deque>
#include "glm/glm.hpp"
#include "GlIdTypes.h"
#include "Utils/GenericConcepts.h"
#include "Math/EngineMath.h"
#include "GlShader.h"

// TODO: maybe have SDrawContext inherit from array<SRenderObjectContainer, MaterialPass::Count> 
// instead of having it as a class member for seamless access?
// Also, it would be sweet to have a custom iterator over the render objects that also provides
// the pipeline state of the object it's iterating over (i.e. it->bIsCCW, it->bIsIndexed, it->bIsTriangle);

struct SDrawArraysCommand
{
	uint32_t Count;
	uint32_t InstanceCount;
	uint32_t FirstIndex;
	uint32_t BaseInstance;
};

struct SDrawElementsCommand
{
	uint32_t Count;
	uint32_t InstanceCount;
	uint32_t FirstIndex;
	int32_t BaseVertex;
	uint32_t BaseInstance;
};

struct SDrawObjectGpuData
{
	SDrawObjectGpuData() = default;
	SDrawObjectGpuData(const SRenderObject& render);

	glm::mat4 RenderTransform {};
	uint32_t HasJoints = false;
	uint32_t JointMatricesBaseIndex = 0;
	uint32_t MaterialIndex = 0;
	int32_t BonesIndexOffset = 0;
};
