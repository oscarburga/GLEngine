#include "SceneGraph.h"

#include "glad/glad.h"
#include "Materials.h"
#include "RenderObject.h"

void SNode::RefreshTransform(const STransform& parentTransform)
{
    WorldTransform = parentTransform * LocalTransform;
    for (auto& child : Children)
        child->RefreshTransform(WorldTransform);
}

void SNode::Draw(const STransform& topTransform, SDrawContext& drawCtx)
{
    for (auto& child : Children)
        child->Draw(topTransform, drawCtx);
}

void SMeshNode::Draw(const STransform& topTransform, SDrawContext& drawCtx)
{
    drawCtx.AddRenderObjects(*this, topTransform);
    SNode::Draw(topTransform, drawCtx);
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
        if (meshPtr->VertexJointsDataBuffer)
            glDeleteBuffers(1, &*meshPtr->VertexJointsDataBuffer);
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

void SLoadedGLTF::Draw(const STransform& topTransform, SDrawContext& drawCtx)
{
    const STransform rootTransform = topTransform * UserTransform;
    for (auto& root : RootNodes)
    {
        root->Draw(rootTransform, drawCtx);
    }
}

void SLoadedGLTF::RefreshNodeTransforms()
{
    for (auto& root : RootNodes)
        root->RefreshTransform();
}
