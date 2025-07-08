#pragma once

#include "Utils/IdType.h"

/*
* Ids representing OpenGL objects (buffers, textures, etc) are prefixed by Gl (i.e. SGlBufferId).
* Ids representing GlEngine objects are not prefixed by Gl (i.e. SMeshId) (likely not using anymore)
*/

/*
* Consider making these Ids "owning", like unique/shared_ptr, so that they call glDelete when destroying.
* May be unnecessary/bad design though, we'll see.
*/
using SGlVaoId = TId<uint32_t, struct VaoIdTag, 0>;
using SGlBufferId = TId<uint32_t, struct BufferIdTag, 0>;
using SGlTextureId = TId<uint32_t, struct TexIdTag, 0>;
using SGlTexArrayId = TId<uint32_t, struct TexArrayIdTag, 0>;
using SGlSamplerId = TId<uint32_t, struct TexIdTag, 0>;
using SGlShaderId = TId<uint32_t, struct ShaderIdTag, 0>;
using SGlFramebufferId = TId<uint32_t, struct FramebufferIdTag, 0>;

struct SGlBufferRangeId : public SGlBufferId
{
	using SGlBufferId::SGlBufferId;
	SGlBufferRangeId(uint32_t id, uint32_t elemSize, size_t head, size_t sizeBytes) : SGlBufferId(id), ElemSize(elemSize), Head(head), SizeBytes(sizeBytes) {};
	uint32_t ElemSize = 1; // Default of 1 to avoid division by zero
	size_t Head = 0;
	size_t SizeBytes = 0;
	size_t GetNumElems() const { assert(SizeBytes % ElemSize == 0);  return SizeBytes / ElemSize; };
	size_t GetHeadInElems() const { assert(Head % ElemSize == 0);  return Head / ElemSize; };
};

namespace GlBindPoints
{
	namespace Ubo
	{
		enum 
		{
			SceneData = 0,
			PbrMaterial = 1,
			JointMatrices = 2,
			Count
		};
	}

	namespace Ssbo
	{
		enum 
		{
			VertexBuffer = 0,
			VertexJointBuffer = 1,
			Count
		};
	}

}

/*
* Uniforms should be mostly reserved for binding textures, and single variables like a model matrix
* (non-array variables and non-struct variables only; use UBO for those cases)
*/
namespace GlUniformLocs 
{
	enum Loc
	{
		ModelMat = 0,

		PbrColorTex = 1,
		PbrMetalRoughTex = 2,
		NormalTex = 3,
		PbrOcclusionTex = 4,

		ShadowDepthTexture = 5,

		HasJoints = 6,
		BonesIndexOffset = 7,

		Count,

		// DEBUG LOCS GO HERE
		DebugIgnoreLighting = 32,
		DebugShowNormals = 33,
		DebugShadowDepthMapIndex = 34,
		DebugCsmTint = 35,
	};
}

namespace GlTexUnits
{
	enum
	{
		PbrColor = 0,
		PhongDiffuse = 0,
		PbrMetalRough = 1,
		PhongSpecular = 1,
		Normal = 2,
		PbrOcclusion = 3,
		Emissive = 4,
		ShadowMap = 5,
		Count
	};
}


// using SMeshId = TId<uint32_t, struct MeshIdTag>;
// using SMaterialId = TId<uint32_t, struct MaterialIdTag>;
// using SPipelineId = TId<uint32_t, struct PipelineIdTag>;
