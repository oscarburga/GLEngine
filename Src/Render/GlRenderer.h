#pragma once

#include <memory>

#include "GlBufferVector.h"
#include "GlShader.h"
#include "Tools/ImguiTools.h"

class CEngine;
class CCsmPipeline;
class CMeshPipeline;
class SGlCamera;

struct SDrawCommands;
struct SDrawContext;
struct SGlTexture;
struct SSceneData;
struct SViewport;

using GlFunctionLoaderFuncType =  void (*)(const char*);

class CGlRenderer : public IImguiObject
{
public:
	static inline CGlRenderer* Renderer = nullptr;

	// TODO: rename, "size" is ambigous (unclear if number of elems or size in bytes)
	// should use something like "Num" for number of elements, and "Bytes" for raw size
	inline static size_t UBOOffsetAlignment = 0;
	inline static size_t UBOMaxBlockSize = 0;
	inline static size_t ShaderMaxMaterialSize = 100;
	inline static size_t DrawDataBufferMaxSize = 100;

	// TODO: Group shader variables into something, don't like them just existing inside the GlRenderer
	inline static const std::string NumCascadesShaderArgName { "NumCascades" };
	inline static const int NumCascades = 4;
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
	SGlBufferVector TexturesSsbo; 
	std::unique_ptr<SGlCamera> ActiveCamera {};
	std::unique_ptr<CMeshPipeline> MeshPipeline {};
	std::unique_ptr<CCsmPipeline> CsmPipeline {};
	std::unique_ptr<SSceneData> SceneData {};
	CGlShader PvpShader { 0 };
	CGlShader QuadShader { 0 };
	static CGlRenderer* Create(GlFunctionLoaderFuncType func);
	void Init(GlFunctionLoaderFuncType func);
	static void Destroy();
	static inline CGlRenderer* Get() { return Renderer; };

	void PrepassDrawDataBuffers();
	void RenderScene(float deltaTime);

	void OnWindowResize(CEngine* Engine, const SViewport& Viewport);

	virtual void ShowImguiPanel() override;
	std::unique_ptr<SDrawContext> MainDrawContext {};

	int32_t RegisterBindlessTexture(const SGlTexture& texture);

	std::vector<uint64_t> TextureHandlesVector;
	SGlBufferVector TextureHandlesBuffer;

	struct 
	{
		bool bShowShadowDepthMap = false;
		bool bDebugCsmTint = false;
		int ShadowDepthMapIndex = 0;
		glm::vec4 SunlightDirection {};
		uint32_t CulledNum = 0;
		uint32_t TotalNum = 0;
	} ImguiData;

private:
	SGlVaoId EmptyVao {};
	SGlBufferId Quad2DBuffer {}; 
	SGlBufferId SceneDataBuffer; // TODO needs double buffering

	std::vector<uint32_t> BlendIndices {}; 
};
