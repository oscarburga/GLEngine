#include "Skinning.h"

#include "GlRenderer.h"
#include "SceneGraph.h"

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
