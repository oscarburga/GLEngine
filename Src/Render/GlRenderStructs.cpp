#include "GlShader.h"
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <fstream>
#include <sstream>
#include <iostream>
#include "GlRenderStructs.h"

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
    {
        STransform nodeTransform = topTransform * WorldTransform;
        glm::mat4 nodeMatrix = nodeTransform.GetMatrix();
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

void SLoadedGLTF::Draw(const STransform& topTransform, SDrawContext& drawCtx)
{
    const STransform rootTransform = topTransform * UserTransform;
    for (auto& root : RootNodes)
    {
        root->Draw(rootTransform, drawCtx);
    }
}

void CAnimator::PlayAnimation(const std::string& anim)
{
    assert(OwnerSkin);
    StopAnimation();
    if (auto it = OwnerSkin->Animations.find(anim); it != OwnerSkin->Animations.end())
    {
        CurrentAnim = &(it->second);
        UpdateAnimation(0.0f);
    }
}

void CAnimator::UpdateAnimation(float deltaTime)
{
    CurrentAnim->JointKeyFrames;
}

void CAnimator::StopAnimation()
{
    if (!CurrentAnim)
        return;
    for (SJointAnimData& jointKeyFrames : CurrentAnim->JointKeyFrames)
        jointKeyFrames.ResetAnimState();
    CurrentAnim = nullptr;

}
