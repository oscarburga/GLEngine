#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "glm/glm.hpp"
#include "GlIdTypes.h"
#include "Utils/GenericConcepts.h"
#include "Math/EngineMath.h"
#include "GlShader.h"

class CAnimator;
struct SDrawContext;
struct SPbrMaterial;

struct STextureAsset
{
	std::string Name;
	SGlTextureId Id;
};

struct SMeshAsset
{
	std::string Name {};
	std::vector<SGeoSurface> Surfaces {};
	SGlBufferRangeId VertexBuffer {};
	SGlBufferRangeId IndexBuffer {};
	SGlBufferRangeId VertexJointsDataBuffer {};
	// TODO: Bones are in their own buffer. Figure out how to include them into the vertex buffer without making the code horrible
	// UPDATE: Do we actually WANT them in the vertex buffer? Separate buffer allows using the same shader without wasting extra space on each vertex on non-skinned meshes.
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
	template<IsCallableWith<SNode*> Func>
	static void TraverseTree(SNode* Node, Func&& f)
	{
		auto travLambda = [&f](SNode* Node) -> void
		{
			f(Node);
			for (auto& child : Node->Children)
			{
				travLambda(child.get());
			}
		};
		travLambda(Node);
	}

};

struct SMeshNode : public SNode
{
	std::shared_ptr<SMeshAsset> Mesh;
	std::shared_ptr<struct SSkinAsset> Skin;
	virtual void Draw(const STransform& topTransform, SDrawContext& drawCtx) override;
};

/*
* BIG TODO: Figure out a better design for separating objects from instances. 
* Currently just trying to get the features I want done asap, but current design doesn't 
* really allow for nicely having multiple instances of the same objects (specially animations, 
* currently using & modifying the actual SAnimationAsset data in-plcae for animations).
*/

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

