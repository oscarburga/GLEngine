#pragma once

#include "GlRenderStructs.h"
#include "GlShader.h"
#include "GlCamera.h"

class CGlShadowDepthPass
{
public:
	~CGlShadowDepthPass();
	static inline std::string NumCascadesShaderArgName { "NumCascades" };
	uint32_t Width = 2048; 
	uint32_t Height = 2048;
	SGlFramebufferId ShadowsFbo {};
	SGlTextureId ShadowsTexture {};
	SGlTexArrayId ShadowsTexArray {};
	CGlShader ShadowsShader { 0 };
	SGlCamera ShadowsCamera {};
	std::vector<SGlCamera> ShadowCameras;
	glm::mat4 LightSpaceMatrix;
	// Split points: first and last points have to be 0.0 and 1.0
	std::vector<float> CascadeSplitPoints = { 0.f, 0.2f , 0.4f, 1.0f }; // TODO set these configurable on imgui

	int GetNumCascades() const { return int(CascadeSplitPoints.size()) - 1; }
	struct
	{
		glm::vec2 OrthoSizeScale { 1.f, 1.f };
		glm::vec2 OrthoSizePadding {};
		uint32_t CulledNum = 0;
		uint32_t TotalNum = 0;
	} ImguiData;

	void Init(uint32_t width = 2048, uint32_t height = 2048);
	void UpdateSceneData(SSceneData& SceneData, const SGlCamera& Camera);
	void RenderShadowDepth(const SSceneData& SceneData, const SDrawContext& DrawContext);
};
