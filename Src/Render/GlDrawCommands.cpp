#include "GlDrawCommands.h"

#include "Materials.h"
#include "Math/Frustum.h"
#include "RenderObject.h"

SDrawObjectGpuData::SDrawObjectGpuData(const SRenderObject& render) :
	RenderTransform(render.RenderTransform),
	HasJoints(render.JointMatricesBuffer && render.VertexJointsDataBuffer),
	JointMatricesBaseIndex((uint32_t)render.JointMatricesBuffer.GetHeadInElems()),
	MaterialIndex((uint32_t)render.Material->DataBuffer.GetHeadInElems()),
	BonesIndexOffset(int32_t(render.VertexJointsDataBuffer.GetHeadInElems<int64_t>() - render.VertexBuffer.GetHeadInElems<int64_t>()))
{};

SDrawCommands::SDrawCommands(size_t MaxDrawDataSize)
{
	DrawDataBuffer = SGlBufferVector(MaxDrawDataSize * sizeof(SDrawObjectGpuData));
	MdiBuffer = SGlBufferVector(MaxDrawDataSize * sizeof(SDrawElementsCommand));
}

void SDrawCommands::ResetBuffers()
{
	IndexedDraws.Reset();
	DrawDataBuffer.Reset();
	MdiBuffer.Reset();
	DrawData.clear();
}

const std::vector<SGlBufferRangeId>& SDrawCommands::GetMdiBufferRanges(bool bCCW) const
{
	return IndexedDraws.MdiRanges[bCCW];
}

uint32_t SDrawCommands::PopulateBuffers(const SRenderObjectContainer& renderObjects, bool bReset, const SCullingFunc& cullingFunc)
{
	if (bReset)
	{
		ResetBuffers();
	}
	uint32_t culledNum = 0;
	size_t beginDrawDataSize = DrawData.size();
	for (int CCW = 0; CCW < 2; CCW++)
	{
		size_t startSz = IndexedDraws.Commands[CCW].size();
		for (const SRenderObject& surface : renderObjects.TriangleObjects[CCW])
		{
			if (cullingFunc(surface))
			{
				++culledNum; 
				continue;
			}
			const uint32_t firstIndex = surface.FirstIndex + (uint32_t)surface.IndexBuffer.GetHeadInElems();
			const int32_t baseVertex = (int32_t)surface.VertexBuffer.GetHeadInElems();
			// SDrawElementsCommand
			IndexedDraws.Commands[CCW].emplace_back(surface.IndexCount, 1, firstIndex, baseVertex, 0);
			// GPU draw data will construct from the SRenderObject
			DrawData.emplace_back(surface);
		}
		if (startSz < IndexedDraws.Commands[CCW].size())
		{
			const size_t numNewElems = IndexedDraws.Commands[CCW].size() - startSz;
			SGlBufferRangeId rangeId = MdiBuffer.Append(numNewElems, IndexedDraws.Commands[CCW].data() + startSz);
			IndexedDraws.MdiRanges[CCW].emplace_back(rangeId);
		}
	}

	if (beginDrawDataSize < DrawData.size())
	{
		const size_t numNewElems = DrawData.size() - beginDrawDataSize;
		DrawDataBuffer.Append(numNewElems, DrawData.data() + beginDrawDataSize);
	}
	return culledNum;
}
