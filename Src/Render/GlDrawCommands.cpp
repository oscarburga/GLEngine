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

uint32_t SDrawCommands::PopulateBuffers(const SRenderObjectContainer& renderObjects, const SFrustum* const CullingFrustum)
{
	IndexedDraws.Reset();
	ArrayDraws.Reset();
	DrawDataBuffer.Reset();
	MdiBuffer.Reset();
	DrawData.clear();

	uint32_t culledNum = 0;
	uint32_t DrawId = 0;
	uint32_t IdxDrawsBeginIdx = 0;
	uint32_t ArrayDrawsBeginIdx = 0;
	for (int indexed = 0; indexed < 2; indexed++)
	{
		for (int CCW = 0; CCW < 2; CCW++)
		{
			for (const SRenderObject& surface : renderObjects.TriangleObjects[CCW][indexed])
			{
				if (surface.Material->UboData.bIgnoreLighting || (CullingFrustum && !CullingFrustum->IsSphereInFrustum(surface.Bounds, surface.WorldTransform)))
				{
					++culledNum;
					continue;
				}
				if (indexed)
				{
					uint32_t firstIndex = surface.FirstIndex + (uint32_t)surface.IndexBuffer.GetHeadInElems();
					int32_t baseVertex = (int32_t)surface.VertexBuffer.GetHeadInElems();
					// SDrawElementsCommand
					IndexedDraws.Commands[CCW].emplace_back(surface.IndexCount, 1, firstIndex, baseVertex, DrawId);
				}
				else
				{
					// SDrawArraysCommand
					ArrayDraws.Commands[CCW].emplace_back(surface.IndexCount, 1, (uint32_t)surface.VertexBuffer.GetHeadInElems(), DrawId);
				}
				// GPU draw data will construct from the SRenderObject
				DrawData.emplace_back(surface);
				++DrawId;
			}
			if (indexed && !IndexedDraws.Commands[CCW].empty())
			{
				IndexedDraws.MdiRanges[CCW] = MdiBuffer.Append(IndexedDraws.Commands[CCW]);
			}
			else if (!indexed && !ArrayDraws.Commands[CCW].empty())
			{
				ArrayDraws.MdiRanges[CCW] = MdiBuffer.Append(ArrayDraws.Commands[CCW]);
			}
		}
	}
	DrawDataBuffer.Append(DrawData);
	return culledNum;
}

const SGlBufferRangeId& SDrawCommands::GetMdiBufferRange(bool bCCW, bool bIndexed) const
{
	return bIndexed ? IndexedDraws.MdiRanges[bCCW] : ArrayDraws.MdiRanges[bCCW];
}
