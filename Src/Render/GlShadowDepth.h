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
	glm::mat4 LightSpaceMatrix;

	void Init(uint32_t width = 2048, uint32_t height = 2048);
	void UpdateSceneData(SSceneData& SceneData);
	void RenderShadowDepth(SSceneData& SceneData, SDrawContext& DrawContext);
};
