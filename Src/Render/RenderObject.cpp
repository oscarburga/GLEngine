#include "RenderObject.h"

#include "glad/glad.h"
#include "GeoSurface.h"
#include "Materials.h"
#include "SceneGraph.h"
#include "Skinning.h"

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

void SDrawContext::Reset()
{
    for (SRenderObjectContainer& renderObjects : RenderObjects)
    {
        renderObjects.ClearAll();
    }
}
