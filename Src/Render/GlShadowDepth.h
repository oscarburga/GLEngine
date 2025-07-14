#pragma once

#include "GlRenderStructs.h"
#include "GlShader.h"
#include "GlCamera.h"
#include <span>

class CGlShadowDepthPass
{
public:
	~CGlShadowDepthPass();
	static inline std::string NumCascadesShaderArgName { "NumCascades" };
	uint32_t Width = 2048; 
	uint32_t Height = 2048;
	SGlFramebufferId ShadowsFbo {};
	SGlTexArrayId ShadowsTexArray {};
	CGlShader ShadowsShader { 0 };
	SGlCamera FullShadowCamera {};
	std::vector<SGlCamera> CascadeCameras;
	glm::mat4 LightSpaceMatrix;
	// Split points: first and last points have to be 0.0 and 1.0
	std::vector<float> CascadeSplitPoints = { 0.f, 0.1f, 0.2f, 0.4f, 1.0f }; // TODO set these configurable on imgui

	struct
	{
		std::span<SDrawElementsCommand> CommandSpans[2] {};
		std::vector<SDrawElementsCommand> Commands {};
	} IndexedDraws {};
	
	struct
	{
		std::span<SDrawArraysCommand> CommandSpans[2] {};
		std::vector<SDrawArraysCommand> Commands {};
	} ArrayDraws {};

	std::vector<SDrawObjectGpuData> DrawData;
	SGlBufferVector DrawDataBuffer;
	SGlBufferVector MdiBuffer;

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
	void PrepassDrawDataBuffer(const SSceneData& SceneData, const SDrawContext& DrawContext);
	void RenderShadowDepth(const SSceneData& SceneData, const SDrawContext& DrawContext);
};
