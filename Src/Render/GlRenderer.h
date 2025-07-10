#pragma once

#include "GlRenderStructs.h"
#include "GlShader.h"
#include "GlCamera.h"
#include "GlShadowDepth.h"
#include "Tools/ImguiTools.h"

class CEngine;
struct SViewport;

typedef void (*GlFunctionLoaderFuncType)(const char*);

class CGlRenderer : public IImguiObject
{
public:
	int UBOOffsetAlignment = 0;
	int UBOMaxBlockSize = 0;
	int ShaderMaxMaterialSize = 100;
	int DrawDataBufferMaxSize = 100;
private:
	CGlRenderer() { bShowImguiPanel = true; };
	CGlRenderer(const CGlRenderer&) = delete;
	CGlRenderer(CGlRenderer&&) = delete;
	~CGlRenderer();
public:
	// TODO: move some of these things, shaders and such into a globally accessible storage
	SGlBufferVector MainVertexBuffer;
	SGlBufferVector MainIndexBuffer;
	SGlBufferVector MainBonesBuffer;
	SGlBufferVector MainMaterialBuffer;
	SGlBufferVector JointMatricesBuffer; // TODO needs double buffering, potentially persistent mapping
	SGlBufferVector DrawDataBuffer; // TODO needs double buffering, potentially persistent mapping
	SGlCamera ActiveCamera {};
	CGlShadowDepthPass ShadowPass {};
	SSceneData SceneData {};
	inline SGlBufferId GetSceneDataUbo() const { return SceneDataBuffer; }
	CGlShader PvpShader { 0 };
	CGlShader QuadShader { 0 };
	static CGlRenderer* Create(GlFunctionLoaderFuncType func);
	void Init(GlFunctionLoaderFuncType func);
	static void Destroy();
	static inline CGlRenderer* Get() { return Renderer; };

	void RenderScene(float deltaTime);

	void OnWindowResize(CEngine* Engine, const SViewport& Viewport);

	virtual void ShowImguiPanel() override;
	SDrawContext MainDrawContext {};
private:
	SGlVaoId EmptyVao {};
	SGlBufferId Quad2DBuffer {};
	SGlBufferId SceneDataBuffer;
	static CGlRenderer* Renderer;
	struct
	{
		bool bShowShadowDepthMap = false;
		bool bDebugCsmTint = false;
		int ShadowDepthMapIndex = 0;
		glm::vec4 SunlightDirection {};
		uint32_t CulledNum = 0;
		uint32_t TotalNum = 0;
	} ImguiData;
};
