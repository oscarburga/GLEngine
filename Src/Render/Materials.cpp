#include "Materials.h"
#include "glad/glad.h"
#include "set"

std::set<uint64_t> AllHandles;

#define TexHandleOp(handle, residentOp) \
	if (handle != 0 && !glIsTextureHandleResidentARB(handle)) {\
		glMakeTextureHandle##residentOp##ARB(handle);\
        AllHandles.insert(handle);\
	}
#define MakeResident(tex) TexHandleOp(tex, Resident)
#define MakeNonResident(tex) TexHandleOp(tex, NonResident)

// void SPbrMaterial::UpdateTextureHandles()
// {
//     UboData.bColorBound = (UboData.ColorTexHandle = ColorTex.GetTextureHandle()) != 0;
//     UboData.bMetalRoughBound = (UboData.MetalRoughTexHandle = MetalRoughTex.GetTextureHandle()) != 0;
//     UboData.bNormalBound = (UboData.NormalTexHandle = NormalTex.GetTextureHandle()) != 0;
//     UboData.bOcclusionBound = (UboData.OcclusionTexHandle = OcclusionTex.GetTextureHandle()) != 0;
// }
// 
// void SPbrMaterial::MakeTextureHandlesResident() const
// {
//     MakeResident(UboData.ColorTexHandle);
//     MakeResident(UboData.MetalRoughTexHandle);
//     MakeResident(UboData.NormalTexHandle);
//     MakeResident(UboData.OcclusionTexHandle);
// }
// 
// void SPbrMaterial::MakeTextureHandlesNonResident() const
// {
//     MakeNonResident(UboData.ColorTexHandle);
//     MakeNonResident(UboData.MetalRoughTexHandle);
//     MakeNonResident(UboData.NormalTexHandle);
//     MakeNonResident(UboData.OcclusionTexHandle);
// }

uint64_t SGlTexture::GetTextureHandle() const
{
    if (Texture)
    {
        if (Sampler)
            return glGetTextureSamplerHandleARB(*Texture, *Sampler);

        return glGetTextureHandleARB(*Texture);
    }
    return 0;
}
