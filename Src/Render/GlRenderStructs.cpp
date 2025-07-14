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

CAnimator::CAnimator(SSkinAsset* ownerSkin) : OwnerSkin(ownerSkin), JointMatrices(ownerSkin->AllJoints.size())
{
    // TODO for performance, persistent-map these (or atleast double-buffer it)
    JointMatricesBuffer = CGlRenderer::Get()->JointMatricesBuffer.Append(JointMatrices);
    UpdateJointMatrices();
}

CAnimator::~CAnimator()
{
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
    CGlRenderer::Get()->JointMatricesBuffer.Update(JointMatricesBuffer, JointMatrices);
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

SGlBufferRangeId SGlBufferVector::AppendRaw(size_t numBytes, const void* pData, uint32_t elemSize)
{
    assert(Id && "Attempting to use AppendRaw on invalid GlBufferVector");
    const bool bEmptyAppend = (numBytes == 0) || !pData;
    const bool bNotEnoughSpace = Head + numBytes > Size;
    if (bEmptyAppend || bNotEnoughSpace)
    {
        std::cout << std::format("AppendRaw failed - buffer={} - bEmptyAppend={} - NotEnoughSpace={}\n", 
            *Id, bEmptyAppend, bNotEnoughSpace);
        return SGlBufferRangeId {};
    }
    SGlBufferRangeId Out { Id, elemSize, Head, numBytes };
    glNamedBufferSubData(*Id, Head, numBytes, pData);
    Head += numBytes;
    return Out;
}

void SGlBufferVector::UpdateRaw(const SGlBufferRangeId& range, size_t numBytes, const void* pData, uint32_t elemSize)
{
    assert(Id && range && (*Id == *range) && "Attempting to use UpdateRaw on invalid GlBufferVector or with invalid BufferRangeId");
    assert((range.SizeBytes == numBytes) && (range.GetNumElems() == (numBytes / elemSize)) && "UpdateRaw invalid in data.");
    const bool bEmptyUpdate = (numBytes == 0) || !pData;
    if (!bEmptyUpdate)
    {
		glNamedBufferSubData(*Id, range.Head, numBytes, pData);
    }
}

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
