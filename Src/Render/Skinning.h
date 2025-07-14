#pragma once

#include <vector>
#include <memory>
#include "GlIdTypes.h"
#include "Utils/GenericConcepts.h"
#include "Math/EngineMath.h"

struct SNode;
struct SSkinAsset;
namespace std { class string; }

template<typename ValueType>
struct SKeyFrame
{
	float Timestamp = 0.0f;
	ValueType Value {};
	template<typename OtherValueType>
	inline bool operator<(const SKeyFrame<OtherValueType>& other) { return Timestamp < other.Timestamp; }
};

template<MatchesAnyType<glm::vec3, glm::quat> ValueType>
struct SAnimKeyFrames : public std::vector<SKeyFrame<ValueType>>
{
private: // For assertion purposes only:
	uint32_t LastIndex = 0;
	float LastTime = 0.0f;
public:

	// EInterpolationType...
	// StepToTime assumes that animTime >= lastAnimTime >= 0.0f. Otherwise asserts.
	ValueType StepToTime(float animTime, const ValueType& DefaultValue)
	{
		assert(LastTime <= animTime);
		LastTime = animTime;

		if (this->size() == 0)
			return DefaultValue;

		while (LastIndex < this->size() && this->at(LastIndex).Timestamp <= animTime)
			++LastIndex;

		// If requested time is less than first keyframe timestamp, grab the value from the earliest keyframe (GLTF spec)
		if (LastIndex == 0 && animTime < this->at(LastIndex).Timestamp)
			return this->at(0).Value;

		// If we reached the end (requested time is past the last keyframe timestamp), grab the value from the last keyframe (GLTF spec)
		if (LastIndex == this->size())
			return this->back().Value;

		// Somewhere in the middle: interpolate
		--LastIndex;
		SKeyFrame<ValueType>& last = this->at(LastIndex);
		SKeyFrame<ValueType>& nxt = this->at(LastIndex + 1);
		const float rangeSize = nxt.Timestamp - last.Timestamp;
		const float timePastLast = animTime - last.Timestamp;
		const float alpha = glm::clamp(timePastLast / rangeSize, 0.f, 1.f);

		// Don't like this too much but w/e, saves us the work of specializing templates
		if constexpr (std::is_same_v<ValueType, glm::vec3>)
			return glm::mix(last.Value, nxt.Value, alpha);

		if constexpr (std::is_same_v<ValueType, glm::quat>)
			return glm::slerp(last.Value, nxt.Value, alpha);
	}
	void ResetAnimState() { LastIndex = 0; LastTime = 0.0f; }
};

struct SVertexSkinData 
{
	glm::uvec4 Joints {};
	glm::vec4 Weights {};
};

struct SJointAnimData
{
	SNode* JointNode = nullptr;
	SAnimKeyFrames<glm::vec3> Positions {};
	SAnimKeyFrames<glm::quat> Rotations {};
	SAnimKeyFrames<glm::vec3> Scales {};

	void ResetAnimState();
	void StepToTime(float animTime);
};

struct SAnimationAsset
{
	float AnimationLength = 0.0f;
	std::vector<SJointAnimData> JointKeyFrames {};
	std::weak_ptr<SSkinAsset> OwnerSkin {};
};

/*
* BIG TODO: Figure out a better design for separating objects from instances. 
* Currently just trying to get the features I want done asap, but current design doesn't 
* really allow for nicely having multiple instances of the same objects (specially animations, 
* currently using & modifying the actual SAnimationAsset data in-plcae for animations).
*/

class CAnimator
{
	friend struct SSkinAsset;
	friend struct SMeshNode;
	float CurrentTime = 0.0f;

private:
	CAnimator(SSkinAsset* ownerSkin);
	struct SSkinAsset* OwnerSkin = nullptr; // Animator is uniquely owned by a skin, so no need for smart ptr
	SAnimationAsset* CurrentAnim = nullptr; // Animations are uniquely owned by the owner skin, so no need for smart ptr
	std::vector<glm::mat4> JointMatrices {};
	bool bLoopCurrentAnim = false;
	SGlBufferRangeId JointMatricesBuffer {};

public:
	~CAnimator();
	bool IsPlaying() { return CurrentAnim != nullptr; }
	void PlayAnimation(const std::string& anim, bool bLoop);
	void UpdateAnimation(float deltaTime);
	void UpdateJointMatrices();
	void StopAnimation();
	inline const SGlBufferRangeId& GetJointMatricesBuffer() { return JointMatricesBuffer; }
};

struct SJoint
{
	uint32_t JointId = 0;
	std::shared_ptr<SNode> Node {};
	glm::mat4 InverseBindMatrix { 1.f };
};
