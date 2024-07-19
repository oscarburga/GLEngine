#pragma once

#include <string>
#include <vector>
#include <array>
#include <unordered_map>
#include "glm/glm.hpp"
#include "GlIdTypes.h"

class CGlShader;

struct SSceneData
{
	glm::vec4 CameraPos {};
	glm::vec4 AmbientColor {};
	glm::vec4 SunlightDirection {}; // 4th component for sun power
	glm::vec4 SunlightColor {};
	glm::mat4 View {};
	glm::mat4 Proj {};
	glm::mat4 ViewProj {};
};

struct SGPUMeshBuffers
{
	SGlBufferId IndexBuffer {};
	SGlBufferId VertexBuffer {};
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
	glm::vec3 Extent;
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
	SGPUMeshBuffers MeshBuffers;
};

struct SRenderObject
{
	uint32_t IndexCount;
	uint32_t FirstIndex;
	SGPUMeshBuffers Buffers;
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
	virtual void Draw(const glm::mat4& topMatrix, SDrawContext& drawCtx) = 0;
};

struct SNode : public IRenderable
{
	std::weak_ptr<SNode> Parent;
	std::vector<std::shared_ptr<SNode>> Children;
	// TODO: Replace these matrices with a transform struct to use quaternions
	glm::mat4 LocalTransform;
	glm::mat4 WorldTransform;
	
	void RefreshTransform(const glm::mat4& parentMatrix);

	virtual void Draw(const glm::mat4& topMatrix, SDrawContext& drawCtx);
};

struct SMeshNode : public SNode
{
	std::shared_ptr<SMeshAsset> Mesh;
	virtual void Draw(const glm::mat4& topMatrix, SDrawContext& drawCtx) override;
};

struct SLoadedGLTF : public IRenderable
{
	std::unordered_map<std::string, std::shared_ptr<SMeshAsset>> Meshes;
	std::unordered_map<std::string, std::shared_ptr<SNode>> Nodes;
	std::unordered_map<std::string, std::shared_ptr<STextureAsset>> Textures;
	std::unordered_map<std::string, std::shared_ptr<SPbrMaterial>> Materials;
	std::vector<SGlSamplerId> Samplers;
	std::vector<std::shared_ptr<SNode>> RootNodes;
	glm::mat4 UserTransform;

	~SLoadedGLTF() { ClearAll(); }
	void ClearAll();
	virtual void Draw(const glm::mat4& topMatrix, SDrawContext& drawCtx) override;
};
