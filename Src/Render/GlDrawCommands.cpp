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
	ArrayDraws.Reset();
	DrawDataBuffer.Reset();
	MdiBuffer.Reset();
	DrawData.clear();
}

const std::vector<SGlBufferRangeId>& SDrawCommands::GetMdiBufferRanges(bool bCCW, bool bIndexed) const
{
	return bIndexed ? IndexedDraws.MdiRanges[bCCW] : ArrayDraws.MdiRanges[bCCW];
}

uint32_t SDrawCommands::PopulateBuffers(const SRenderObjectContainer& renderObjects, bool bReset, const SCullingFunc& cullingFunc)
{
	if (bReset)
	{
		ResetBuffers();
	}
	uint32_t culledNum = 0;
	size_t beginDrawDataSize = DrawData.size();
	uint32_t drawId = (uint32_t)beginDrawDataSize;
	for (int indexed = 0; indexed < 2; indexed++)
	{
		for (int CCW = 0; CCW < 2; CCW++)
		{
			size_t startSz = indexed ? IndexedDraws.Commands[CCW].size() : ArrayDraws.Commands[CCW].size();
			for (const SRenderObject& surface : renderObjects.TriangleObjects[CCW][indexed])
			{
				if (cullingFunc(surface))
				{
					++culledNum; 
					continue;
				}
				if (indexed)
				{
					const uint32_t firstIndex = surface.FirstIndex + (uint32_t)surface.IndexBuffer.GetHeadInElems();
					const int32_t baseVertex = (int32_t)surface.VertexBuffer.GetHeadInElems();
					// SDrawElementsCommand
					IndexedDraws.Commands[CCW].emplace_back(surface.IndexCount, 1, firstIndex, baseVertex, drawId);
				}
				else
				{
					// SDrawArraysCommand
					ArrayDraws.Commands[CCW].emplace_back(surface.IndexCount, 1, (uint32_t)surface.VertexBuffer.GetHeadInElems(), drawId);
				}
				// GPU draw data will construct from the SRenderObject
				DrawData.emplace_back(surface);
				++drawId;
			}
			if (indexed && startSz < IndexedDraws.Commands[CCW].size())
			{
				const size_t numNewElems = IndexedDraws.Commands[CCW].size() - startSz;
				SGlBufferRangeId rangeId = MdiBuffer.Append(numNewElems, IndexedDraws.Commands[CCW].data() + startSz);
				IndexedDraws.MdiRanges[CCW].emplace_back(rangeId);
			}
			else if (!indexed && !ArrayDraws.Commands[CCW].empty())
			{
				const size_t numNewElems = ArrayDraws.Commands[CCW].size() - startSz;
				SGlBufferRangeId rangeId = MdiBuffer.Append(numNewElems, ArrayDraws.Commands[CCW].data() + startSz);
				ArrayDraws.MdiRanges[CCW].emplace_back(rangeId);
			}
		}
	}
	if (beginDrawDataSize < DrawData.size())
	{
		const size_t numNewElems = DrawData.size() - beginDrawDataSize;
		DrawDataBuffer.Append(numNewElems, DrawData.data() + beginDrawDataSize);
	}
	return culledNum;
}
