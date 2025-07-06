#include "GlShader.h"
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <format>
#include <fstream>
#include <sstream>
#include <iostream>
#include "GlRenderStructs.h"
#include <Utils/ForEachIndexed.h>

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
	// If we're using animations/skinning, send the model matrix as just the top transform. 
	// World transforms are already built into the joints.
	const bool bIsSkinned = Skin && Skin->Animator;
	STransform nodeTransform = topTransform * WorldTransform;
	glm::mat4 nodeMatrix = nodeTransform.GetMatrix();

	const bool bIsCCW = glm::determinant(nodeMatrix) > 0.f; 
	for (auto& surface : Mesh->Surfaces)
	{
		SRenderObject& draw = drawCtx.Surfaces[surface.Material->MaterialPass].emplace_back();
		draw.bIsCCW = bIsCCW;
		draw.IndexCount = surface.Count;
		draw.FirstIndex = surface.StartIndex;
		draw.Bounds = surface.Bounds;
		draw.Material = surface.Material;
		draw.VertexBuffer = Mesh->VertexBuffer;
		draw.IndexBuffer = Mesh->IndexBuffer;
		draw.VertexJointsDataBuffer = Mesh->VertexJointsDataBuffer;
		draw.JointMatricesBuffer = bIsSkinned ? Skin->Animator->JointMatricesBuffer : SGlBufferId {};
		draw.WorldTransform = nodeMatrix;
		draw.RenderTransform = bIsSkinned ? topTransform.GetMatrix() : nodeMatrix;
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

CAnimator::CAnimator(SSkinAsset* ownerSkin) : OwnerSkin(ownerSkin), JointMatrices(ownerSkin->AllJoints.size())
{
    glCreateBuffers(1, &*JointMatricesBuffer);
    glNamedBufferStorage(*JointMatricesBuffer, JointMatrices.size() * sizeof(glm::mat4), nullptr, GL_DYNAMIC_STORAGE_BIT);
    UpdateJointMatrices();
}

CAnimator::~CAnimator()
{
    glDeleteBuffers(1, &*JointMatricesBuffer);
}

void CAnimator::PlayAnimation(const std::string& anim, bool bLoop)
{
    assert(JointMatricesBuffer);
    assert(OwnerSkin);
    StopAnimation();
    bLoopCurrentAnim = bLoop;
    if (auto it = OwnerSkin->Animations.find(anim); it != OwnerSkin->Animations.end())
    {
        CurrentAnim = &(it->second);
        UpdateAnimation(0.0f);
    }
}

void CAnimator::UpdateAnimation(float deltaTime)
{
    if (!CurrentAnim || !JointMatricesBuffer)
        return;

    CurrentTime += deltaTime;
    bool bNeedsReset = false;
    if (CurrentTime > CurrentAnim->AnimationLength)
    {
        CurrentTime = std::fmod(CurrentTime, CurrentAnim->AnimationLength);
        bNeedsReset = true;
    }

    for (SJointAnimData& jointKeyFrames : CurrentAnim->JointKeyFrames)
    {
        if (bNeedsReset)
            jointKeyFrames.ResetAnimState();

        jointKeyFrames.StepToTime(CurrentTime);
    }

    if (OwnerSkin->SkeletonRoot)
    {
        auto parent = OwnerSkin->SkeletonRoot->Parent.lock();
        OwnerSkin->SkeletonRoot->RefreshTransform(parent ? parent->WorldTransform : STransform {});
    }
	// UGLY FALLBACK IF THERE'S NO SKELETON ROOT PROVIDED (and I CBA manually finding it) very slow
    else for (auto& joint : OwnerSkin->AllJoints)
    {
        auto parent = joint.Node->Parent.lock();
        joint.Node->RefreshTransform(parent ? parent->WorldTransform : STransform {});
    }
    UpdateJointMatrices();
}

void CAnimator::UpdateJointMatrices()
{
	for (auto& joint : OwnerSkin->AllJoints)
	{
		JointMatrices[joint.JointId] = joint.Node->WorldTransform.GetMatrix() * joint.InverseBindMatrix;
	}
	glNamedBufferSubData(*JointMatricesBuffer, 0, JointMatrices.size() * sizeof(glm::mat4), JointMatrices.data());
}

void CAnimator::StopAnimation()
{
    if (!CurrentAnim)
        return;
    for (SJointAnimData& jointKeyFrames : CurrentAnim->JointKeyFrames)
        jointKeyFrames.ResetAnimState();
    CurrentAnim = nullptr;

}

void SJointAnimData::ResetAnimState()
{
    Positions.ResetAnimState();
    Rotations.ResetAnimState();
    Scales.ResetAnimState();
    if (JointNode)
        JointNode->LocalTransform = JointNode->OriginalLocalTransform;
}

void SJointAnimData::StepToTime(float animTime)
{
    if (!JointNode)
        return;

    // If a property is not modified at all (i.e. positions/scales/rotations are empty), 
    // must keep the original node's transform

    const STransform& jointOriginalTransform = JointNode->OriginalLocalTransform;

    glm::vec3 pos = Positions.StepToTime(animTime, jointOriginalTransform.GetPosition());
    glm::vec3 scale = Scales.StepToTime(animTime, jointOriginalTransform.GetScale());
    glm::quat rot = Rotations.StepToTime(animTime, jointOriginalTransform.GetRotation());
    JointNode->LocalTransform = STransform { pos, rot, scale };
}

void SSkinAsset::InitAnimator()
{
    assert(AllJoints.size());
    // Animator = std::make_unique<CAnimator>(this, maxJoints);
    Animator = std::unique_ptr<CAnimator>(new CAnimator(this));
}

SGlBufferVector::SGlBufferVector(size_t size) : Size(size), Head(0)
{
    glCreateBuffers(1, &*Id);
    glNamedBufferStorage(*Id, size, nullptr, GL_DYNAMIC_STORAGE_BIT);
}

SGlBufferVector::~SGlBufferVector()
{
    if (Id)
		glDeleteBuffers(1, &*Id);
}

SGlBufferVector::SGlBufferVector(SGlBufferVector&& Other)
{
    // Writing it like this to make it EXTRA explicit we're move-assigning this.
    operator=(std::move(Other));
}

SGlBufferVector& SGlBufferVector::operator=(SGlBufferVector&& Other)
{
    // just copy and memset for simplicity, dirty but will do for now
    memcpy(this, &Other, sizeof(SGlBufferVector));
    memset(&Other, 0, sizeof(SGlBufferVector));
    return *this;
}

SGlBufferRangeId SGlBufferVector::AppendRaw(size_t numBytes, const void* data, uint32_t elemSize)
{
    assert(Id && "Attempting to use AppendRaw on invalid GlBufferVector");
    if (Head + numBytes > Size)
    {
        std::cout << std::format("AppendRaw not enough space remaining for buffer {}!! - Head = {}, Size = {}, numBytes = {}", *Id, Head, Size, numBytes);
        return SGlBufferRangeId {};
    }
    SGlBufferRangeId Out { Id, elemSize, Head, numBytes };
    glNamedBufferSubData(*Id, Head, numBytes, data);
    Head += numBytes;
    return Out;
}
