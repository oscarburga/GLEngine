#include "GlShader.h"
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <format>
#include <fstream>
#include <sstream>
#include <iostream>
#include "GlRenderStructs.h"
#include <Utils/ForEachIndexed.h>
#include "GlRenderer.h"


void SPbrMaterial::UpdateTextureHandles()
{
    UboData.ColorTexHandle = ColorTex.GetTextureHandle();
    UboData.MetalRoughTexHandle = MetalRoughTex.GetTextureHandle();
    UboData.NormalTexHandle = NormalTex.GetTextureHandle();
    UboData.OcclusionTexHandle = OcclusionTex.GetTextureHandle();
}

#define TexHandleOp(tex, residentOp) \
	if (uint64_t handle = tex.GetTextureHandle()) \
		glMakeTextureHandle##residentOp##ARB(handle)
#define MakeResident(tex) TexHandleOp(tex, Resident)
#define MakeNonResident(tex) TexHandleOp(tex, NonResident)

void SPbrMaterial::MakeTextureHandlesResident() const
{
    MakeResident(ColorTex);
    MakeResident(MetalRoughTex);
    MakeResident(NormalTex);
    MakeResident(OcclusionTex);
}

void SPbrMaterial::MakeTextureHandlesNonResident() const
{
    MakeNonResident(ColorTex);
    MakeNonResident(MetalRoughTex);
    MakeNonResident(NormalTex);
    MakeNonResident(OcclusionTex);
}

uint64_t SGlTexture::GetTextureHandle() const
{
    // leaving commented for now so renderdoc doesn't kapoot
    // if (Texture)
    // {
    //     if (Sampler)
    //         return glGetTextureSamplerHandleARB(*Texture, *Sampler);

    //     return glGetTextureHandleARB(*Texture);
    // }
    return 0;
}
