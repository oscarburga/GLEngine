#pragma once

#include "Utils/IdType.h"

/*
* Ids representing OpenGL objects (buffers, textures, etc) are prefixed by Gl (i.e. SGlBufferId).
* Ids representing GlEngine objects are not prefixed by Gl (i.e. SMeshId).
*/

/*
* Consider maki these Ids "owning", like unique/shared_ptr, so that they call glDelete when destroying.
* May be unnecessary though, we'll see.
*/
using SGlBufferId = TId<uint32_t, struct BufferIdTag, 0>;
using SGlTextureId = TId<uint32_t, struct TexIdTag, 0>;
using SGlSamplerId = TId<uint32_t, struct TexIdTag, 0>;
using SGlShaderId = TId<uint32_t, struct ShaderIdTag, 0>;

// using SMeshId = TId<uint32_t, struct MeshIdTag>;
// using SMaterialId = TId<uint32_t, struct MaterialIdTag>;
// using SPipelineId = TId<uint32_t, struct PipelineIdTag>;
