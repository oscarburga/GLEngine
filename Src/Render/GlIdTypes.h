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
using SGlSamplerId = TId<uint32_t, struct TexIdTag, 0>;
using SGlShaderId = TId<uint32_t, struct ShaderIdTag, 0>;
using SGlFramebufferId = TId<uint32_t, struct FbIdTag, 0>;

struct SGlBufferRangeId : public SGlBufferId
{
	uint32_t StartOffset = 0;
	uint32_t Size = 0;
};

namespace GlBindPoints
{
	namespace Ubo
	{
		enum 
		{
			SceneData = 0,
			PbrMaterial = 1,
			Count
		};
	}

	namespace Ssbo
	{
		enum 
		{
			VertexBuffer = 0,
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
		PhongDiffuseTex = 1,

		PbrMetalRoughTex = 2,
		PhongSpecularTex = 2,

		NormalTex = 3,
		PhongShininess = 3,

		OcclusionTex = 4,

		Count,

		// DEBUG LOCS GO HERE
		DebugIgnoreLighting = 32,
		ShowDebugNormals = 33
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
		Count
	};
}


// using SMeshId = TId<uint32_t, struct MeshIdTag>;
// using SMaterialId = TId<uint32_t, struct MaterialIdTag>;
// using SPipelineId = TId<uint32_t, struct PipelineIdTag>;
