#pragma once

#include <memory>

#include "GlBufferVector.h"
#include "GlShader.h"
#include "Tools/ImguiTools.h"

class CEngine;
class CGlShadowDepthPass;
class SGlCamera;

struct SDrawCommands;
struct SDrawContext;
struct SGlTexture;
struct SSceneData;
struct SViewport;

typedef void (*GlFunctionLoaderFuncType)(const char*);

class CGlRenderer : public IImguiObject
{
public:
	// TODO: rename, "size" is ambigous (unclear if number of elems or size in bytes)
	// should use something like "Num" for number of elements, and "Bytes" for raw size
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
	SGlBufferVector TexturesSsbo; // TODO needs double buffering, potentially persistent mapping
	std::unique_ptr<SGlCamera> ActiveCamera {};
	std::unique_ptr<CGlShadowDepthPass> ShadowPass {};
	std::unique_ptr<SSceneData> SceneData {};
	std::unique_ptr<SDrawCommands> DrawCommands {};
	inline SGlBufferId GetSceneDataUbo() const { return SceneDataBuffer; }
	CGlShader PvpShader { 0 };
	CGlShader QuadShader { 0 };
	static CGlRenderer* Create(GlFunctionLoaderFuncType func);
	void Init(GlFunctionLoaderFuncType func);
	static void Destroy();
	static inline CGlRenderer* Get() { return Renderer; };

	void PrepassDrawDataBuffer();
	void RenderScene(float deltaTime);

	void OnWindowResize(CEngine* Engine, const SViewport& Viewport);

	virtual void ShowImguiPanel() override;
	std::unique_ptr<SDrawContext> MainDrawContext {};

	int32_t RegisterTexture(const SGlTexture& texture);

	std::vector<uint64_t> TextureHandlesVector;
	SGlBufferVector TextureHandlesBuffer;

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
