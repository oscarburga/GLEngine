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

	SShaderLoadArgs vsArgs("Shaders/pvpDepthPassCSM.vert", { { NumCascadesShaderArgName, std::to_string(numCascades) } });
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

void CGlShadowDepthPass::RenderShadowDepth(const SSceneData& SceneData, const SDrawContext& DrawContext)
{
	glViewport(0, 0, Width, Height);
	glBindFramebuffer(GL_FRAMEBUFFER, *ShadowsFbo);
	glClear(GL_DEPTH_BUFFER_BIT);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);

	SFrustum shadowsFrustum;
	FullShadowCamera.CalcFrustum(&shadowsFrustum, nullptr);
	ShadowsShader.Use();
	ImguiData.TotalNum = (uint32_t)DrawContext.Surfaces[EMaterialPass::MainColor].size();
	ImguiData.CulledNum = 0;
	for (auto& surface : DrawContext.Surfaces[EMaterialPass::MainColor])
	{
		if (!shadowsFrustum.IsSphereInFrustum(surface.Bounds, surface.WorldTransform) || surface.Material->bIgnoreLighting)
		{
			++ImguiData.CulledNum;
			continue;
		}
		ShadowsShader.SetUniform(GlUniformLocs::ModelMat, surface.RenderTransform);
		ShadowsShader.SetUniform(GlUniformLocs::HasJoints, surface.VertexJointsDataBuffer && surface.JointMatricesBuffer);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, GlBindPoints::Ssbo::VertexBuffer, surface.VertexBuffer);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, GlBindPoints::Ssbo::VertexJointBuffer, surface.VertexJointsDataBuffer);
		glBindBufferBase(GL_UNIFORM_BUFFER, GlBindPoints::Ubo::JointMatrices, surface.JointMatricesBuffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, surface.IndexBuffer);

		// TODO: there's something wrong somewhere with the face culling / winding order, should not need to invert this...
		// Figure it out at some point
		constexpr int windingOrder[] = { GL_CW, GL_CCW }; 
		glFrontFace(windingOrder[!surface.bIsCCW]); ;

		if (surface.IndexBuffer)
			glDrawElements(surface.Material->PrimitiveType, surface.IndexCount, GL_UNSIGNED_INT, (void*)static_cast<uint64_t>(surface.FirstIndex));
		else
			glDrawArrays(surface.Material->PrimitiveType, surface.FirstIndex, surface.IndexCount);
	}

	// Restore opengl viewport size to window size
	auto& viewport = CEngine::Get()->Viewport;
	glViewport(0, 0, viewport.SizeX, viewport.SizeY);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glUseProgram(0);
}
