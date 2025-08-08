#pragma once

#include <span>

#include "glm/glm.hpp"

#include "GlBufferVector.h"
#include "GlIdTypes.h"
#include <functional>

struct SFrustum;
struct SRenderObject;
struct SRenderObjectContainer;

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

	struct SDrawCommandSpans
	{
		std::vector<SGlBufferRangeId> MdiRanges[2] {};
		std::vector<SDrawElementsCommand> Commands[2] {};
		void Reset() 
		{ 
			MdiRanges[0].clear();
			MdiRanges[1].clear();
			Commands[0].clear();
			Commands[1].clear();
		}
	};

	SDrawCommandSpans IndexedDraws {};
	std::vector<SDrawObjectGpuData> DrawData {};
	SGlBufferVector DrawDataBuffer {};
	SGlBufferVector MdiBuffer {};

	void ResetBuffers();
	const std::vector<SGlBufferRangeId>& GetMdiBufferRanges(bool bCCW) const;

	// TODO: template this culling func or worst case scenario use an enum-and-switch for it. 
	// Type-erased funcs are slowwww for repeat calls.
	using SCullingFunc = std::function<bool(const SRenderObject&)>;
	uint32_t PopulateBuffers(const SRenderObjectContainer& renderObjects, bool bReset, const SCullingFunc& cullingFunc);
};
