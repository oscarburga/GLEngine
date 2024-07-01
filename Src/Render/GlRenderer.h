#pragma once

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
	static CGlRenderer* Create(GlFunctionLoaderFuncType func);
	void Init(GlFunctionLoaderFuncType func);
	static void Destroy();
	static inline CGlRenderer* Get() { return Renderer; };

	void RenderScene(float deltaTime);

	void OnWindowResize(CEngine* Engine, const SViewport& Viewport);

private:
	static CGlRenderer* Renderer;
};
