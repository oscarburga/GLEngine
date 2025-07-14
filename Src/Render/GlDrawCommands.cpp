#include "GlShader.h"
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <format>
#include <fstream>
#include <sstream>
#include <iostream>
#include "GlRenderStructs.h"
#include <Utils/ForEachIndexed.h>
#include "GlRenderer.h"

SDrawObjectGpuData::SDrawObjectGpuData(const SRenderObject& render) :
	RenderTransform(render.RenderTransform),
	HasJoints(render.JointMatricesBuffer && render.VertexJointsDataBuffer),
	JointMatricesBaseIndex((uint32_t)render.JointMatricesBuffer.GetHeadInElems()),
	MaterialIndex((uint32_t)render.Material->DataBuffer.GetHeadInElems()),
	BonesIndexOffset(int32_t(render.VertexJointsDataBuffer.GetHeadInElems<int64_t>() - render.VertexBuffer.GetHeadInElems<int64_t>()))
{};

SRenderObject::SRenderObject(bool bIsCCW, const SGeoSurface& surface, const SMeshNode& meshNode, const STransform& topTransform, const glm::mat4& nodeMatrix)
    : IndexCount(surface.Count), FirstIndex(surface.StartIndex), Bounds(surface.Bounds), bIsCCW(bIsCCW), WorldTransform(nodeMatrix), Material(surface.Material)
{
    const bool bIsSkinned = meshNode.Skin && meshNode.Skin->Animator;
    const SMeshAsset& asset = *meshNode.Mesh;
    VertexBuffer = asset.VertexBuffer;
    IndexBuffer = asset.IndexBuffer;
    VertexJointsDataBuffer = asset.VertexJointsDataBuffer;
    JointMatricesBuffer = bIsSkinned ? meshNode.Skin->Animator->GetJointMatricesBuffer() : SGlBufferRangeId {};
    RenderTransform = bIsSkinned ? topTransform.GetMatrix() : nodeMatrix;
}

void SRenderObjectContainer::ClearAll()
{
    for (int i = 0; i < 2; i++)
    {
        for (int j = 0; j < 2; j++)
        {
            TriangleObjects[i][j].clear();
        }
    }
    OtherObjects.clear();
    TotalSize = 0;
}

void SDrawContext::AddRenderObjects(const SMeshNode& meshNode, const STransform& topTransform)
{
	STransform nodeTransform = topTransform * meshNode.WorldTransform;
	glm::mat4 nodeMatrix = nodeTransform.GetMatrix();

    // TODO: I believe this determinant should actually be > 0, not < 0. 
    // Anyways, it works for now and its not a priority to understand why right now. Revisit later.
    const bool bIsCCW = glm::determinant(nodeMatrix) > 0.f; 
    bool bIsIndexedDraw = meshNode.Mesh->IndexBuffer.IsValid();
    for (SGeoSurface& surface : meshNode.Mesh->Surfaces)
    {
        const EMaterialPass::Pass materialPass = surface.Material->MaterialPass;
        uint32_t primitive = surface.Material->PrimitiveType;
        SRenderObjectContainer& container = RenderObjects[materialPass];
        if (primitive == GL_TRIANGLES && materialPass != EMaterialPass::Transparent)
        {
            container.TriangleObjects[bIsCCW][bIsIndexedDraw].emplace_back(bIsCCW, surface, meshNode, topTransform, nodeMatrix);
        }
        else
        {
            container.OtherObjects.emplace_back(bIsCCW, surface, meshNode, topTransform, nodeMatrix);
        }
        ++container.TotalSize;
    }
}

void SPbrMaterial::UpdateTextureHandles()
{
    UboData.ColorTexHandle = ColorTex.GetTextureHandle();
    UboData.MetalRoughTexHandle = MetalRoughTex.GetTextureHandle();
    UboData.NormalTexHandle = NormalTex.GetTextureHandle();
    UboData.OcclusionTexHandle = OcclusionTex.GetTextureHandle();
}

#define TexHandleOp(tex, residentOp) \
	if (uint64_t handle = tex.GetTextureHandle()) \
		glMakeTextureHandle##residentOp##ARB(handle)
#define MakeResident(tex) TexHandleOp(tex, Resident)
#define MakeNonResident(tex) TexHandleOp(tex, NonResident)

void SPbrMaterial::MakeTextureHandlesResident() const
{
    MakeResident(ColorTex);
    MakeResident(MetalRoughTex);
    MakeResident(NormalTex);
    MakeResident(OcclusionTex);
}

void SPbrMaterial::MakeTextureHandlesNonResident() const
{
    MakeNonResident(ColorTex);
    MakeNonResident(MetalRoughTex);
    MakeNonResident(NormalTex);
    MakeNonResident(OcclusionTex);
}

uint64_t SGlTexture::GetTextureHandle() const
{
    // leaving commented for now so renderdoc doesn't kapoot
    // if (Texture)
    // {
    //     if (Sampler)
    //         return glGetTextureSamplerHandleARB(*Texture, *Sampler);

    //     return glGetTextureHandleARB(*Texture);
    // }
    return 0;
}
