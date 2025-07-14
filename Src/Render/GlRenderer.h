#pragma once

#include <memory>

#include "GlBufferVector.h"
#include "GlShader.h"
#include "Tools/ImguiTools.h"

class CEngine;
class CGlShadowDepthPass;
class SGlCamera;

struct SViewport;
struct SSceneData;
struct SDrawContext;

typedef void (*GlFunctionLoaderFuncType)(const char*);

class CGlRenderer : public IImguiObject
{
public:
	inline static size_t UBOOffsetAlignment = 0;
	inline static size_t UBOMaxBlockSize = 0;
	inline static size_t ShaderMaxMaterialSize = 100;
	inline static size_t DrawDataBufferMaxSize = 100;
private:
	CGlRenderer();;
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
	std::unique_ptr<SGlCamera> ActiveCamera {};
	std::unique_ptr<CGlShadowDepthPass> ShadowPass {};
	std::unique_ptr<SSceneData> SceneData {};
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
	std::unique_ptr<SDrawContext> MainDrawContext {};
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

	std::vector<uint32_t> BlendIndices {}; 
};
