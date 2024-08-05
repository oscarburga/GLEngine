#pragma once

#include "GlRenderStructs.h"
#include "GlShader.h"
#include "GlCamera.h"

class CGlShadowDepthPass
{
public:
	~CGlShadowDepthPass();
	uint32_t Width = 2048; 
	uint32_t Height = 2048;
	SGlFramebufferId ShadowsFbo {};
	SGlTextureId ShadowsTexture {};
	CGlShader ShadowsShader { 0 };
	SGlCamera ShadowsCamera {};
	glm::mat4 LightSpaceMatrix;

	struct
	{
		glm::vec2 OrthoSizeScale { 1.f, 1.f };
		glm::vec2 OrthoSizePadding {};
	} ImguiData;

	void Init(uint32_t width = 2048, uint32_t height = 2048);
	void UpdateSceneData(SSceneData& SceneData, const SGlCamera& Camera);
	void RenderShadowDepth(const SSceneData& SceneData, const SDrawContext& DrawContext);
};
