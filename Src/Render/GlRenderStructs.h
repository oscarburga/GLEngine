#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include "glm/glm.hpp"
#include "GlIdTypes.h"

class CGlShader;

struct SGPUMeshBuffers
{
	SGlBufferId IndexBuffer {};
	SGlBufferId VertexBuffer {};
};

struct SGPUTexture
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
	// Temp: utility for easier test setup
	void SetUniforms(CGlShader& shader);
};

struct STexturedMaterial 
{
	SGPUTexture Diffuse = {};
	SGPUTexture Specular = {};
	float Shininess = 32.f;
};

struct SGeoSurface 
{
	uint32_t StartIndex = 0;
	uint32_t Count = 0;
	// Temp: this eventually needs to be replaced by a templated material or something
	STexturedMaterial Material = {}; 
};

// template<typename Base, typename Derived>
// concept IsBaseOf = std::is_base_of_v<Base, Derived>;

// struct SMaterialPipeline 
// { 
// 	SMaterialId Material;
// 	SShaderId Shader;
// };

struct SVertex
{
	glm::vec3 Position = {};
	float uv_x = 0.0f;
	glm::vec3 Normal = {};
	float uv_y = 0.0f;
	glm::vec4 Color = {};
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
	SGPUMeshBuffers MeshBuffers;
};


class IRenderable
{
	virtual void Draw(const glm::mat4& topMatrix) = 0;
};

struct SNode : public IRenderable
{
	std::weak_ptr<SNode> Parent;
	std::vector<std::shared_ptr<SNode>> Children;
	// TODO: Replace these matrices with a transform struct to use quaternions
	glm::mat4 LocalTransform;
	glm::mat4 WorldTransform;
	
	void RefreshTransform(const glm::mat4& parentMatrix)
	{
		WorldTransform = parentMatrix * LocalTransform;
		for (auto& child : Children)
			child->RefreshTransform(parentMatrix);
	}

	virtual void Draw(const glm::mat4& topMatrix)
	{
		for (auto& child : Children)
			child->Draw(topMatrix);
	}
};

struct SMeshNode : public SNode
{
	std::shared_ptr<SMeshAsset> Mesh;
	virtual void Draw(const glm::mat4& topMatrix) override;
};

struct SLoadedGLTF : public IRenderable
{
	std::unordered_map<std::string, std::shared_ptr<SMeshAsset>> Meshes;
	std::unordered_map<std::string, std::shared_ptr<SNode>> Nodes;
	std::unordered_map<std::string, std::shared_ptr<STextureAsset>> Textures;
	std::vector<SGlSamplerId> Samplers;
	std::vector<std::shared_ptr<SNode>> RootNodes;
	// TODO: Materials
	//std::unordered_map<std::string, std::shared_ptr<SGLTFMaterial>> Materials;
	//SGlBufferId MaterialDataBuffer

	~SLoadedGLTF() { ClearAll(); }
	void ClearAll();
	virtual void Draw(const glm::mat4& topMatrix) override {} // TODO
};

struct SGLTFMaterial; // TODO
