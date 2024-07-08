#pragma once

#include "GlRenderStructs.h"
#include "GlShader.h"

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
	CGlShader PvpShaderTextured { 0 };
	static CGlRenderer* Create(GlFunctionLoaderFuncType func);
	void Init(GlFunctionLoaderFuncType func);
	static void Destroy();
	static inline CGlRenderer* Get() { return Renderer; };

	void RenderScene(float deltaTime);

	void OnWindowResize(CEngine* Engine, const SViewport& Viewport);

	SDrawContext MainDrawContext {};
private:
	SGlVaoId EmptyVao {};
	static CGlRenderer* Renderer;
};
