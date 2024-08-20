#include "GlShader.h"
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <fstream>
#include <sstream>
#include <iostream>
#include "GlRenderStructs.h"

void SNode::RefreshTransform(const glm::mat4& parentMatrix)
{
    WorldTransform = parentMatrix * LocalTransform;
    for (auto& child : Children)
        child->RefreshTransform(WorldTransform);
}

void SNode::Draw(const glm::mat4& topMatrix, SDrawContext& drawCtx)
{
    for (auto& child : Children)
        child->Draw(topMatrix, drawCtx);
}

void SMeshNode::Draw(const glm::mat4& topMatrix, SDrawContext& drawCtx)
{
    {
        glm::mat4 nodeMatrix = topMatrix * WorldTransform;
        for (auto& surface : Mesh->Surfaces)
        {
            auto& draw = drawCtx.Surfaces[surface.Material->MaterialPass].emplace_back();
            draw.IndexCount = surface.Count;
            draw.FirstIndex = surface.StartIndex;
            draw.Bounds = surface.Bounds;
            draw.Material = surface.Material;
            draw.VertexBuffer = Mesh->VertexBuffer;
            draw.IndexBuffer = Mesh->IndexBuffer;
            draw.BonesDataBuffer = Mesh->BoneDataBuffer;
            draw.Transform = nodeMatrix;
        }
    }
	SNode::Draw(topMatrix, drawCtx);
}

void SLoadedGLTF::ClearAll()
{
    // This might be a bad idea... its fine if noone else is using buffers/etc from this graph though.
    for (auto& [name, meshPtr] : Meshes)
    {
        if (meshPtr->IndexBuffer)
            glDeleteBuffers(1, &*meshPtr->IndexBuffer);
        if (meshPtr->VertexBuffer)
            glDeleteBuffers(1, &*meshPtr->VertexBuffer);
        if (meshPtr->BoneDataBuffer)
            glDeleteBuffers(1, &*meshPtr->BoneDataBuffer);
    }
    Meshes.clear();

    // Delete samplers
    static_assert(sizeof(SGlSamplerId) == sizeof(uint32_t));
    glDeleteSamplers((GLsizei)Samplers.size(), reinterpret_cast<uint32_t*>(Samplers.data()));
    Samplers.clear();

    // Delete textures
    for (auto texId : Textures)
    {
        if (texId)
            glDeleteTextures(1, &*texId);
    }
    Textures.clear();

    // Delete materials
    for (auto& [name, matPtr] : Materials)
    {
        if (matPtr->DataBuffer)
            glDeleteBuffers(1, &*matPtr->DataBuffer);
    }

}

void SLoadedGLTF::Draw(const glm::mat4& topMatrix, SDrawContext& drawCtx)
{
    const glm::mat4 rootTransform = UserTransform * topMatrix;
    for (auto& root : RootNodes)
    {
        root->Draw(rootTransform, drawCtx);
    }
}

