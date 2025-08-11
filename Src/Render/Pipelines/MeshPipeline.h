#pragma once

#include "Render/GlDrawCommands.h"
#include "Render/GlShader.h"

#include <memory>

class CGlRenderer;

class SGlCamera;
struct SSceneData;
struct SDrawCommands;
struct SDrawContext;

class CMeshPipeline
{
	CGlShader PvpShader { 0 };
	std::unique_ptr<SDrawCommands> DrawCommands {};

public:

	void Init();
	void UpdateSceneData(SSceneData& SceneData, const SGlCamera& Camera);
	void PrepassDrawDataBuffer(CGlRenderer& renderer);
	void Render(const CGlRenderer& renderer);

};
