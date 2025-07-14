#include <glad/glad.h>
#include "Engine.h"
#include "Math/EngineMath.h"
#include "GlShadowDepth.h"
#include "Assets/AssetLoader.h"
#include "GlRenderer.h"
#include <iostream>

CGlShadowDepthPass::~CGlShadowDepthPass()
{
	if (ShadowsFbo)
		glDeleteFramebuffers(1, &*ShadowsFbo);
	if (ShadowsTexArray)
		glDeleteTextures(1, &*ShadowsTexArray);
	if (ShadowsShader.Id)
		glDeleteProgram(ShadowsShader.Id);
}

void CGlShadowDepthPass::Init(uint32_t width, uint32_t height)
{
	FullShadowCamera.bIsPerspective = false;
	Width = width;
	Height = height;

	const int numCascades = GetNumCascades();

	// Shadow map array for CSM
	CascadeCameras.resize(numCascades);
	std::for_each(CascadeCameras.begin(), CascadeCameras.end(), [&](SGlCamera& shadowCamera)
	{
		shadowCamera.bIsPerspective = false;
	});
	assert(numCascades >= 1);
	assert(CascadeSplitPoints.front() == 0.0f);
	assert(CascadeSplitPoints.back() == 1.f);
	assert(std::is_sorted(CascadeSplitPoints.begin(), CascadeSplitPoints.end()));

	glCreateTextures(GL_TEXTURE_2D_ARRAY, 1, &*ShadowsTexArray);
	glTextureStorage3D(*ShadowsTexArray, 1, GL_DEPTH_COMPONENT32F, width, height, numCascades);

	glTextureParameteri(*ShadowsTexArray, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTextureParameteri(*ShadowsTexArray, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTextureParameteri(*ShadowsTexArray, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTextureParameteri(*ShadowsTexArray, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	constexpr float borderColor[] = { 1.f, 1.f, 1.f, 1.f };
	glTextureParameterfv(*ShadowsTexArray, GL_TEXTURE_BORDER_COLOR, borderColor);
	glTextureParameteri(*ShadowsTexArray, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
	glTextureParameteri(*ShadowsTexArray, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);

	// Setup framebuffer
	glCreateFramebuffers(1, &*ShadowsFbo);
	glNamedFramebufferTexture(*ShadowsFbo, GL_DEPTH_ATTACHMENT, *ShadowsTexArray, 0);
	glNamedFramebufferDrawBuffer(*ShadowsFbo, GL_NONE);
	glNamedFramebufferReadBuffer(*ShadowsFbo, GL_NONE);

	assert(glCheckNamedFramebufferStatus(*ShadowsFbo, GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);

	DrawDataBuffer = SGlBufferVector(CGlRenderer::DrawDataBufferMaxSize * sizeof(SDrawObjectGpuData));
	MdiBuffer = SGlBufferVector(CGlRenderer::DrawDataBufferMaxSize * sizeof(SDrawElementsCommand));
	IndexedDraws.Commands.reserve(CGlRenderer::DrawDataBufferMaxSize);
	ArrayDraws.Commands.reserve(CGlRenderer::DrawDataBufferMaxSize);
	SShaderLoadArgs vsArgs("Shaders/pvpDepthPassCSM.vert", { { NumCascadesShaderArgName, std::to_string(numCascades) } });
	vsArgs.SetArg("MAX_DRAWS", CGlRenderer::DrawDataBufferMaxSize);
	SShaderLoadArgs gsArgs("Shaders/pvpDepthPassCSM.geom", { { NumCascadesShaderArgName, std::to_string(numCascades) } });
	auto shader = CAssetLoader::LoadShaderProgram(vsArgs, gsArgs, "Shaders/empty.frag");
	assert(shader);

	ShadowsShader = *shader;

}

void CGlShadowDepthPass::UpdateSceneData(SSceneData& SceneData, const SGlCamera& Camera)
{
	std::array<vec3, 8> frustumCorners;
	Camera.CalcFrustum(nullptr, &frustumCorners);
	const vec3 shadowsCamDir = vec3(SceneData.SunlightDirection);
	
	const vec3 shadowsUp = [&]
	{
		const vec3 shadowsRight = glm::normalize(glm::cross(shadowsCamDir, World::Up));
		// If sunlight points straight up/down, just set the shadowsUp to be World::Front
		if (glm::any(glm::isnan(shadowsRight)) || (glm::length2(shadowsRight) <= 1e-8))
		{
			return World::Front;
		}
		return glm::normalize(glm::cross(shadowsRight, shadowsCamDir));
	}(); 

	auto CalcCascade = [&](int index) // Pass a negative index to calculate the full camera
	{
		auto [fromPercent, toPercent, ShadowsCamera] = [this, index]() -> std::tuple<const float, const float, SGlCamera&>
		{
			assert(index + 1 < int(CascadeSplitPoints.size()));
			if (index >= 0)
			{
				const float fromPercent = CascadeSplitPoints[index];
				const float toPercent = CascadeSplitPoints[index + 1];
				return { fromPercent, toPercent, CascadeCameras[index] };
			}
			else
			{
				return { 0.0f, 1.f, FullShadowCamera };
			}
		}();
		
		std::array<vec3, 8> subFrustumCorners;
		for (int i = 0; i < 4; i++)
		{
			const vec3 dir = frustumCorners[i + 4] - frustumCorners[i];
			subFrustumCorners[i] = frustumCorners[i] + (fromPercent * dir);
			subFrustumCorners[i + 4] = frustumCorners[i] + (toPercent * dir);
		}

		const vec3 frustumCenter = std::accumulate(subFrustumCorners.begin(), subFrustumCorners.end(), glm::vec3 { 0.f }) / 8.f;
		ShadowsCamera.Position = frustumCenter - shadowsCamDir;
		const mat4 lightView = glm::lookAt(ShadowsCamera.Position, frustumCenter, shadowsUp);
		ShadowsCamera.Rotation = glm::lookAtRotation(shadowsCamDir, shadowsUp);
		glm::vec3 min { std::numeric_limits<float>::max() };
		glm::vec3 max { std::numeric_limits<float>::min() };
		for (auto& corner : subFrustumCorners)
		{
			const vec3 cornerLVspace = vec3(lightView * vec4(corner, 1.0f));
			min = glm::min(min, cornerLVspace);
			max = glm::max(max, cornerLVspace);
		}
		min *= vec3 { ImguiData.OrthoSizeScale, 1.f };
		max *= vec3 { ImguiData.OrthoSizeScale, 1.f };
		min -= vec3 { ImguiData.OrthoSizePadding, 0.0f };
		max += vec3 { ImguiData.OrthoSizePadding, 0.0f };
		ShadowsCamera.OrthoMinBounds = vec2 { min };
		ShadowsCamera.OrthoMaxBounds = vec2 { max };
		ShadowsCamera.NearPlane = -max.z; // min.z;
		ShadowsCamera.FarPlane = -min.z; // max.z;
		
		if (index >= 0)
		{
			SceneData.CascadeDistances[index] = vec4(Camera.FarPlane * toPercent, 0.0f, 0.0f, 0.0f);
			mat4 lightProj;
			ShadowsCamera.CalcProjMatrix(lightProj);
			SceneData.LightSpaceTransforms[index] = lightProj * lightView;
		}
	};

	//  Yes, start from -1 to calculate the full shadows camera, then all the cascades
	for (int i = -1, lim = GetNumCascades(); i < lim; i++)
	{
		CalcCascade(i);
	}
}

void CGlShadowDepthPass::PrepassDrawDataBuffer(const SSceneData& SceneData, const SDrawContext& DrawContext)
{
	IndexedDraws.Commands.clear();
	IndexedDraws.CommandSpans[0] = IndexedDraws.CommandSpans[1] = {};

	ArrayDraws.Commands.clear();
	ArrayDraws.CommandSpans[0] = ArrayDraws.CommandSpans[1] = {};

	DrawData.clear();
	DrawDataBuffer.Reset();

	ImguiData.TotalNum = (uint32_t)DrawContext.RenderObjects[EMaterialPass::MainColor].TotalSize;
	ImguiData.CulledNum = 0;
	SFrustum shadowsFrustum;
	FullShadowCamera.CalcFrustum(&shadowsFrustum, nullptr);
	const SRenderObjectContainer& renderObjects = DrawContext.RenderObjects[EMaterialPass::MainColor];

	uint32_t DrawId = 0;
	uint32_t IdxDrawsBeginIdx = 0;
	uint32_t ArrayDrawsBeginIdx = 0;
	for (int indexed = 0; indexed < 2; indexed++)
	{
		for (int CCW = 0; CCW < 2; CCW++)
		{
			for (const SRenderObject& surface : renderObjects.TriangleObjects[CCW][indexed])
			{
				if (!shadowsFrustum.IsSphereInFrustum(surface.Bounds, surface.WorldTransform) || surface.Material->UboData.bIgnoreLighting)
				{
					++ImguiData.CulledNum;
					continue;
				}
				if (indexed)
				{
					GLuint firstIndex = surface.FirstIndex + (GLuint)surface.IndexBuffer.GetHeadInElems();
					GLint baseVertex = (GLint)surface.VertexBuffer.GetHeadInElems();
					// SDrawElementsCommand
					IndexedDraws.Commands.emplace_back(surface.IndexCount, 1, firstIndex, baseVertex, DrawId);
				}
				else
				{
					// SDrawArraysCommand
					ArrayDraws.Commands.emplace_back(surface.IndexCount, 1, (GLuint)surface.VertexBuffer.GetHeadInElems(), DrawId);
				}
				// GPU draw data will construct from the SRenderObject
				DrawData.emplace_back(surface);
				++DrawId;
			}
			if (indexed)
			{
				IndexedDraws.CommandSpans[CCW] = { IndexedDraws.Commands.begin() + IdxDrawsBeginIdx, IndexedDraws.Commands.end() };
				IdxDrawsBeginIdx = (uint32_t)IndexedDraws.Commands.size();
			}
			else
			{
				ArrayDraws.CommandSpans[CCW] = { ArrayDraws.Commands.begin() + ArrayDrawsBeginIdx, ArrayDraws.Commands.end() };
				ArrayDrawsBeginIdx = (uint32_t)ArrayDraws.Commands.size();
			}
		}
	}
	DrawDataBuffer.Append(DrawData);
}

void CGlShadowDepthPass::RenderShadowDepth(const SSceneData& SceneData, const SDrawContext& DrawContext)
{
	PrepassDrawDataBuffer(SceneData, DrawContext);
	glEnable(GL_DEPTH_TEST);
	glViewport(0, 0, Width, Height);
	glBindFramebuffer(GL_FRAMEBUFFER, *ShadowsFbo);
	glClear(GL_DEPTH_BUFFER_BIT);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);

	SFrustum shadowsFrustum;
	FullShadowCamera.CalcFrustum(&shadowsFrustum, nullptr);
	ShadowsShader.Use();
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, CGlRenderer::Get()->MainIndexBuffer.Id);
	glBindBuffer(GL_DRAW_INDIRECT_BUFFER, MdiBuffer.Id);
	glBindBufferBase(GL_UNIFORM_BUFFER, GlBindPoints::Ubo::JointMatrices, CGlRenderer::Get()->JointMatricesBuffer.Id);
	glBindBufferBase(GL_UNIFORM_BUFFER, GlBindPoints::Ubo::DrawData, DrawDataBuffer.Id);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, GlBindPoints::Ssbo::VertexBuffer, CGlRenderer::Get()->MainVertexBuffer.Id);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, GlBindPoints::Ssbo::VertexJointBuffer, CGlRenderer::Get()->MainBonesBuffer.Id);

	const SRenderObjectContainer& renderObjects = DrawContext.RenderObjects[EMaterialPass::MainColor];
	for (int CCW = 0; CCW < 2; CCW++)
	{
		constexpr int windingOrder[] = { GL_CW, GL_CCW }; 
		glFrontFace(windingOrder[CCW ^ 1]); // culling backface, so also need to flip this

		if (IndexedDraws.CommandSpans[CCW].size())
		{
			// ShadowsShader.SetUniform(GlUniformLocs::BaseDrawId, (int)IndexedDraws.CommandSpans[CCW].front().BaseInstance);
			auto rangeId = MdiBuffer.Append(IndexedDraws.CommandSpans[CCW].size(), IndexedDraws.CommandSpans[CCW].data());
			glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, nullptr, rangeId.GetNumElems(), 0);
			MdiBuffer.Reset();
		}

		if (ArrayDraws.CommandSpans[CCW].size())
		{
			glMultiDrawArraysIndirect(GL_TRIANGLES, ArrayDraws.CommandSpans[CCW].data(), ArrayDraws.CommandSpans[CCW].size(), 0);
		}
	}

	// Restore opengl viewport size to window size
	auto& viewport = CEngine::Get()->Viewport;
	glViewport(0, 0, viewport.SizeX, viewport.SizeY);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glUseProgram(0);
}
