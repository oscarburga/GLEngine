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

struct SGlOffsetBuffer : public SGlBufferId
{
	uint32_t Offset = 0;
};

namespace GlBindPoints
{
	namespace Ubo
	{
		enum 
		{
			SceneData = 0,
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

	namespace Tex // TODO
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
}


// using SMeshId = TId<uint32_t, struct MeshIdTag>;
// using SMaterialId = TId<uint32_t, struct MaterialIdTag>;
// using SPipelineId = TId<uint32_t, struct PipelineIdTag>;
