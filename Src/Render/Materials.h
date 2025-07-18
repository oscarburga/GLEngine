#pragma once

#include <string>

#include "GlIdTypes.h"
#include "Math/EngineMath.h"

// TODO: maybe SGlTexture needs its own header? prob not.
struct SGlTexture
{
	SGlTextureId Texture {};
	SGlSamplerId Sampler {};
	uint64_t GetTextureHandle() const;
};

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
	int32_t ColorTexIndex = -1;
	int32_t MetalRoughTexIndex = -1;
	int32_t NormalTexIndex = -1;
	int32_t OcclusionTexIndex = -1;
	// uint32_t bColorBound = false;
	// uint32_t bMetalRoughBound = false;
	// uint32_t bNormalBound = false;
	// uint32_t bOcclusionBound = false;
	uint32_t bIgnoreLighting = false;
	uint32_t _padding[2] = {}; // Padding for std140
	// uint64_t ColorTexHandle = 0;
	// uint64_t MetalRoughTexHandle = 0;
	// uint64_t NormalTexHandle = 0;
	// uint64_t OcclusionTexHandle = 0;
};

struct SPbrMaterial
{
	EMaterialPass::Pass MaterialPass {};
	uint32_t PrimitiveType = 4; // GL_TRIANGLES
	std::string Name {};
	SGlTexture ColorTex {}; // Also known as "Albedo"
	SGlTexture MetalRoughTex {}; // Metal in blue channel, Roughness in green.
	SGlTexture NormalTex {};
	SGlTexture OcclusionTex {};
	// SGPUTexture EmissiveTex {};
	SGlBufferRangeId DataBuffer {};
	SPbrMaterialUboData UboData {};

	void UpdateTextureHandles();
	void MakeTextureHandlesResident() const;
	void MakeTextureHandlesNonResident() const;
};
