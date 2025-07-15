#pragma once

#include <span>

#include "glm/glm.hpp"

#include "GlBufferVector.h"
#include "GlIdTypes.h"

struct SFrustum;
struct SRenderObject;
struct SRenderObjectContainer;

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

// TODO double buffer
struct SDrawCommands
{
	SDrawCommands() = default;
	SDrawCommands(size_t MaxDrawDataSize);

	template<typename T>
	struct TDrawCommandSpans
	{
		SGlBufferRangeId MdiRanges[2] {};
		std::vector<T> Commands[2] {};
		void Reset() 
		{ 
			MdiRanges[0] = MdiRanges[1] = {};
			Commands[0].clear();
			Commands[1].clear();
		}
	};

	TDrawCommandSpans<SDrawElementsCommand> IndexedDraws {};
	TDrawCommandSpans<SDrawArraysCommand> ArrayDraws {};
	std::vector<SDrawObjectGpuData> DrawData {};
	SGlBufferVector DrawDataBuffer {};
	SGlBufferVector MdiBuffer {};

	uint32_t PopulateBuffers(const SRenderObjectContainer& drawCtx, const SFrustum* const CullingFrustum);
	const SGlBufferRangeId& GetMdiBufferRange(bool bCCW, bool bIndexed) const;
};
