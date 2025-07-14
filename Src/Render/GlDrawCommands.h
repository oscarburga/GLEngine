#pragma once

#include <string>
#include <vector>
#include <array>
#include <unordered_map>
#include <deque>
#include "glm/glm.hpp"
#include "GlIdTypes.h"
#include "Utils/GenericConcepts.h"
#include "Math/EngineMath.h"
#include "GlShader.h"

struct SGeoSurface;

struct SRenderObject // TODO: some bCastShadows bool
{
public:
	SRenderObject() = default;
	SRenderObject(bool bIsCCW, const SGeoSurface& surface, const SMeshNode& meshNode, const STransform& topTransform, const glm::mat4& nodeMatrix);
	SGlBufferRangeId VertexBuffer;
	SGlBufferRangeId IndexBuffer;
	SGlBufferRangeId VertexJointsDataBuffer;
	SGlBufferRangeId JointMatricesBuffer;
	// We could just use a SGeoSurface here instead of storing IndexCount,FirstIndex, Bounds, Material (its the same data)
	// Unfortunately if we do that, bIsCCW bool would destroy the struct alignment.
	uint32_t IndexCount;
	uint32_t FirstIndex;
	SBounds Bounds;
	bool bIsCCW = false;
	glm::mat4 WorldTransform; // For frustum culling (may differ on skinned meshes from the render transform)
	glm::mat4 RenderTransform; // For actual rendering
	std::shared_ptr<SPbrMaterial> Material;
};

struct SRenderObjectContainer
{
	// TODO: a more robust, generic & flexible solution.
	// This also doesn't allow proper sorting for blended-objects (I need them rendered in order
	// of distance, I don't implement OIT). Current workaround is just toss all blendable objects 
	// into OtherObjects unbatched rendering.  

	// For now, 4 vectors will work since the only pipeline state we change is glFrontFace.
	// TriangleObjects[bIsCCW][bIsIndexedDraw]
	std::vector<SRenderObject> TriangleObjects[2][2]; // Batch multidraws for GL_TRIANGLE surfaces
	std::vector<SRenderObject> OtherObjects; // Single-draws for non GL_TRIANGLE surfaces (and all blendable objects, for now)

	size_t TotalSize = 0;
	void ClearAll();
};

// TODO: maybe have SDrawContext inherit from array<SRenderObjectContainer, MaterialPass::Count> 
// instead of having it as a class member for seamless access?
// Also, it would be sweet to have a custom iterator over the render objects that also provides
// the pipeline state of the object it's iterating over (i.e. it->bIsCCW, it->bIsIndexed, it->bIsTriangle);

struct SMeshNode;
struct SDrawContext
{
	std::array<SRenderObjectContainer, EMaterialPass::Count> RenderObjects;
	void AddRenderObjects(const SMeshNode& meshNode, const STransform& topTransform);
};

struct SDrawArraysCommand
{
	uint32_t Count;
	uint32_t InstanceCount;
	uint32_t FirstIndex;
	uint32_t BaseInstance;
};

struct SDrawElementsCommand
{
	uint32_t Count;
	uint32_t InstanceCount;
	uint32_t FirstIndex;
	int32_t BaseVertex;
	uint32_t BaseInstance;
};

struct SDrawObjectGpuData
{
	SDrawObjectGpuData() = default;
	SDrawObjectGpuData(const SRenderObject& render);

	glm::mat4 RenderTransform {};
	uint32_t HasJoints = false;
	uint32_t JointMatricesBaseIndex = 0;
	uint32_t MaterialIndex = 0;
	int32_t BonesIndexOffset = 0;
};
