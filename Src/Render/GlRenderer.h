#pragma once

#include "GlRenderStructs.h"
#include "GlShader.h"
#include "GlCamera.h"
#include "GlShadowDepth.h"

class CEngine;
struct SViewport;

typedef void (*GlFunctionLoaderFuncType)(const char*);

class CGlRenderer
{
	CGlRenderer() {}
	CGlRenderer(const CGlRenderer&) = delete;
	CGlRenderer(CGlRenderer&&) = delete;
	~CGlRenderer();
public:
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

	SDrawContext MainDrawContext {};
private:
	SGlVaoId EmptyVao {};
	SGlBufferId Quad2DBuffer {};
	SGlBufferId SceneDataBuffer;
	static CGlRenderer* Renderer;
public:
	static int UBOOffsetAlignment;
};
