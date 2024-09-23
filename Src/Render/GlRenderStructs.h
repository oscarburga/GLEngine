#pragma once

#include <string>
#include <vector>
#include <array>
#include <unordered_map>
#include "glm/glm.hpp"
#include "GlIdTypes.h"
#include "Utils/GenericConcepts.h"
#include "Math/EngineMath.h"

class CGlShader;

struct SSceneData
{
	glm::vec4 CameraPos {};
	glm::vec4 SunlightDirection {}; // 4th component for sun power
	glm::vec4 SunlightColor {};
	glm::mat4 View {};
	glm::mat4 Proj {};
	glm::mat4 ViewProj {};
	glm::mat4 LightSpaceTransform {};
};

struct SGlTexture
{
	SGlTextureId Texture {};
	SGlSamplerId Sampler {};
};

struct SSolidMaterial 
{
	glm::vec3 Ambient {};
	glm::vec3 Diffuse {};
	glm::vec3 Specular {};
	float Shininess = 32.f;
};

struct STexturedMaterial 
{
	bool bIgnoreLighting = false; 
	uint32_t PrimitiveType = 4; // GL_TRIANGLES
	SGlTexture Diffuse = {};
	SGlTexture Specular = {};
	float Shininess = 32.f;
};

namespace EMaterialPass
{
	enum Pass : uint8_t
	{
		First,
		MainColor = First,
		MainColorMasked,
		Transparent,
		Count
	};
}

struct SPbrMaterialUboData
{
	glm::vec4 ColorFactor { 1.f, 1.f, 1.f, 1.f };
	float MetalFactor = 1.f;
	float RoughFactor = 1.f;
	float AlphaCutoff = 0.f;
	float NormalScale = 1.0f;
	float OcclusionStrength = 0.0f;
	// glm::vec3 EmissiveFactor {};
	// TODO: Merge all of these into a bitmask so we don't have to pass so much data to the GPU (each bool in std140 GPU is 4 bytes)
	uint32_t bColorBound = false;
	uint32_t bMetalRoughBound = false;
	uint32_t bNormalBound = false;
	uint32_t bOcclusionBound = false;
	// bool bNormalBound = false;
	// bool bOcclusionBound = false;
	// bool bEmissiveBound = false;
	// bool bPadding[3] = {};
	// int ColorTextureUnit = GlBindPoints::Tex::PbrColor;
	// int MetalRoughTextureUnit = GlBindPoints::Tex::PbrMetalRough;
	// int NormalTextureUnit = GlBindPoints::Tex::Normal;
	// int OcclusionTextureUnit = GlBindPoints::Tex::PbrOcclusion;
	// int EmissiveTextureUnit = GlBindPoints::Tex::PbrOcclusion;
};

struct SPbrMaterial
{
	bool bIgnoreLighting = false;
	EMaterialPass::Pass MaterialPass {};
	uint32_t PrimitiveType = 4; // GL_TRIANGLES
	std::string Name {};
	SGlTexture ColorTex {}; // Also known as "Albedo"
	SGlTexture MetalRoughTex {}; // Metal in blue channel, Roughness in green.
	SGlTexture NormalTex {};
	SGlTexture OcclusionTex {};
	// SGPUTexture EmissiveTex {};
	SGlBufferId DataBuffer {};
	SPbrMaterialUboData UboData {};
};

struct SBounds
{
	float Radius; 
	glm::vec3 Origin;
	glm::vec3 Extent; // Extent is the offset from the middle to opposite bounding box corners
};

struct SGeoSurface 
{
	uint32_t StartIndex = 0;
	uint32_t Count = 0;
	SBounds Bounds {};
	std::shared_ptr<SPbrMaterial> Material {};
};

struct SVertex
{
	glm::vec3 Position = {};
	float uv_x = 0.0f;
	glm::vec3 Normal = {};
	float uv_y = 0.0f;
	glm::vec4 Color = {};
	glm::vec4 Tangent = {}; // xyz normalized, W is the sign (+-1) indicating tangent handedness
};

struct STextureAsset
{
	std::string Name;
	SGlTextureId Id;
};

struct SMeshAsset
{
	std::string Name;
	std::vector<SGeoSurface> Surfaces;
	SGlBufferId VertexBuffer;
	SGlBufferId IndexBuffer;
	SGlBufferId VertexJointsDataBuffer; // TODO: Bones are in their own buffer. Figure out how to include them into the vertex buffer without making the code horrible
};

struct SRenderObject // TODO: some bCastShadows bool
{
	uint32_t IndexCount;
	uint32_t FirstIndex;
	SGlBufferId VertexBuffer;
	SGlBufferId IndexBuffer;
	SGlBufferId VertexJointsDataBuffer;
	SGlBufferId JointMatricesBuffer;
	SBounds Bounds;
	glm::mat4 Transform;
	std::shared_ptr<SPbrMaterial> Material;
};

struct SDrawContext
{
	std::vector<size_t> BlendIndices;
	std::array<std::vector<SRenderObject>, EMaterialPass::Count> Surfaces;
};

class IRenderable
{
	virtual void Draw(const STransform& topTransform, SDrawContext& drawCtx) = 0;
};

struct SNode : public IRenderable
{
	uint32_t NodeId = 0;
	std::weak_ptr<SNode> Parent;
	std::vector<std::shared_ptr<SNode>> Children;
	STransform LocalTransform;
	STransform WorldTransform;
	STransform OriginalLocalTransform;

	void RefreshTransform(const STransform& topTransform = STransform {});
	virtual void Draw(const STransform& topTransform, SDrawContext& drawCtx) override;
	// glm::mat4 LocalTransform;
	// glm::mat4 WorldTransform;
	
	// void RefreshTransform(const glm::mat4& parentMatrix);
	// virtual void Draw(const glm::mat4& topMatrix, SDrawContext& drawCtx);
};

struct SMeshNode : public SNode
{
	std::shared_ptr<SMeshAsset> Mesh;
	std::shared_ptr<struct SSkinAsset> Skin;
	virtual void Draw(const STransform& topTransform, SDrawContext& drawCtx) override;
};

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
	std::weak_ptr<struct SSkinAsset> OwnerSkin {};
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
	CAnimator(SSkinAsset* ownerSkin, uint32_t maxJointId);
	struct SSkinAsset* OwnerSkin = nullptr; // Animator is uniquely owned by a skin, so no need for smart ptr
	SAnimationAsset* CurrentAnim = nullptr; // Animations are uniquely owned by the owner skin, so no need for smart ptr
	std::vector<glm::mat4> JointMatrices {};
	bool bLoopCurrentAnim = false;
	SGlBufferId JointMatricesBuffer {};

public:
	~CAnimator();
	bool IsPlaying() { return CurrentAnim != nullptr; }
	void PlayAnimation(const std::string& anim, bool bLoop);
	void UpdateAnimation(float deltaTime);
	void StopAnimation();

};

struct SJoint
{
	uint32_t JointId = 0;
	std::shared_ptr<SNode> Node {};
	glm::mat4 InverseBindMatrix { 1.f };
};

struct SSkinAsset
{
	std::string Name;
	std::vector<SJoint> AllJoints;
	std::shared_ptr<SNode> SkeletonRoot;
	std::unordered_map<std::string, SAnimationAsset> Animations;
	std::unique_ptr<CAnimator> Animator = nullptr;
	
	void InitAnimator(uint32_t maxJoints);
};

struct SLoadedGLTF : public IRenderable
{
	std::unordered_map<std::string, std::shared_ptr<SMeshAsset>> Meshes;
	std::unordered_map<std::string, std::shared_ptr<SNode>> Nodes;
	// std::unordered_map<std::string, std::shared_ptr<STextureAsset>> Textures; // Don't think I need them mapped by name for now
	std::unordered_map<std::string, std::shared_ptr<SPbrMaterial>> Materials; // Possibly don't need these mapped by name either
	std::unordered_map<std::string, std::shared_ptr<SSkinAsset>> Skins;
	std::vector<std::string> AnimationNames;
	std::vector<SGlTextureId> Textures;
	std::vector<SGlSamplerId> Samplers;
	std::vector<std::shared_ptr<SNode>> RootNodes;
	STransform UserTransform;

	~SLoadedGLTF() { ClearAll(); }
	void ClearAll();
	virtual void Draw(const STransform& topTransform, SDrawContext& drawCtx) override;
	void RefreshNodeTransforms();
};

