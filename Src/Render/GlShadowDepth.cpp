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
	if (ShadowsTexture)
		glDeleteTextures(1, &*ShadowsTexture);
	if (ShadowsShader.Id)
		glDeleteProgram(ShadowsShader.Id);
}

void CGlShadowDepthPass::Init(uint32_t width, uint32_t height)
{
	ShadowsCamera.bIsPerspective = false;
	Width = width;
	Height = height;
	glCreateFramebuffers(1, &*ShadowsFbo);
	glCreateTextures(GL_TEXTURE_2D, 1, &*ShadowsTexture);
	glTextureStorage2D(*ShadowsTexture, 1, GL_DEPTH_COMPONENT32F, width, height);
	// glTextureParameteri(*ShadowsTexture, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	// glTextureParameteri(*ShadowsTexture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTextureParameteri(*ShadowsTexture, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTextureParameteri(*ShadowsTexture, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTextureParameteri(*ShadowsTexture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTextureParameteri(*ShadowsTexture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float borderColor[] = { 1.f, 1.f, 1.f, 1.f };
	glTextureParameterfv(*ShadowsTexture, GL_TEXTURE_BORDER_COLOR, borderColor);

	glNamedFramebufferTexture(*ShadowsFbo, GL_DEPTH_ATTACHMENT, *ShadowsTexture, 0);
	glNamedFramebufferDrawBuffer(*ShadowsFbo, GL_NONE);
	glNamedFramebufferReadBuffer(*ShadowsFbo, GL_NONE);

	auto shader = CAssetLoader::LoadShaderProgram("Shaders/pvpShadows.vert", "Shaders/empty.frag");
	assert(shader);
	ShadowsShader = *shader;
}

void CGlShadowDepthPass::UpdateSceneData(SSceneData& SceneData, const SGlCamera& Camera)
{
	vec3 mainCameraFront = glm::rotateByQuat(World::Front, Camera.Rotation);
	std::array<vec3, 8> frustumCorners;
	Camera.CalcFrustum(nullptr, &frustumCorners);
	const vec3 frustumCenter = std::accumulate(frustumCorners.begin(), frustumCorners.end(), vec3{0.0f}) / 8.f;

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
	ShadowsCamera.Position = frustumCenter - shadowsCamDir;
	const mat4 lightView = glm::lookAt(ShadowsCamera.Position, frustumCenter, shadowsUp);
	ShadowsCamera.Rotation = glm::quat_cast(lightView);
	glm::vec3 min { std::numeric_limits<float>::max() };
	glm::vec3 max { std::numeric_limits<float>::min() };
	for (auto& corner : frustumCorners)
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
	ShadowsCamera.NearPlane = min.z;
	ShadowsCamera.FarPlane = max.z;
	mat4 lightProj;
	ShadowsCamera.CalcProjMatrix(lightProj);
	SceneData.LightSpaceTransform = lightProj * lightView;
}

void CGlShadowDepthPass::RenderShadowDepth(const SSceneData& SceneData, const SDrawContext& DrawContext)
{
	glViewport(0, 0, Width, Height);
	glBindFramebuffer(GL_FRAMEBUFFER, *ShadowsFbo);
	glClear(GL_DEPTH_BUFFER_BIT);

	SFrustum shadowsFrustum;
	ShadowsCamera.CalcFrustum(&shadowsFrustum, nullptr);
	ShadowsShader.Use();
	ImguiData.TotalNum = (uint32_t)DrawContext.Surfaces[EMaterialPass::MainColor].size();
	ImguiData.CulledNum = 0;
	for (auto& surface : DrawContext.Surfaces[EMaterialPass::MainColor])
	{
		if (!shadowsFrustum.IsSphereInFrustum(surface.Bounds, surface.Transform) || surface.Material->bIgnoreLighting)
		{
			++ImguiData.CulledNum;
			continue;
		}
		ShadowsShader.SetUniform(GlUniformLocs::ModelMat, surface.Transform);
		ShadowsShader.SetUniform(GlUniformLocs::HasJoints, surface.VertexJointsDataBuffer && surface.JointMatricesBuffer);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, GlBindPoints::Ssbo::VertexBuffer, surface.VertexBuffer);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, GlBindPoints::Ssbo::VertexJointBuffer, surface.VertexJointsDataBuffer);
		glBindBufferBase(GL_UNIFORM_BUFFER, GlBindPoints::Ubo::JointMatrices, surface.JointMatricesBuffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, surface.IndexBuffer);
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
