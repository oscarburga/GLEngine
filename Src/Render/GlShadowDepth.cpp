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
	SFrustum camFrustum;
	std::array<vec3, 8> frustumCorners;
	Camera.CalcFrustum(&camFrustum, &frustumCorners);
	const vec3 frustumCenter = std::accumulate(frustumCorners.begin(), frustumCorners.end(), vec3{0.0f}) / 8.f;

	const vec3 shadowsCamDir = vec3(SceneData.SunlightDirection);
	const vec3 shadowsRight = glm::normalize(glm::cross(shadowsCamDir, World::Up));
	const vec3 shadowsUp = glm::normalize(glm::cross(shadowsRight, shadowsCamDir));

	glm::vec3 min(std::numeric_limits<float>::max()), max(std::numeric_limits<float>::min());
	ShadowsCamera.Position = frustumCenter - shadowsCamDir;
	// const mat4 lightView = glm::lookAt(ShadowsCamera.Position, frustumCenter, shadowsUp);
	const mat4 lightView = glm::lookAt(ShadowsCamera.Position, frustumCenter, World::Up);
	ShadowsCamera.Rotation = glm::quat_cast(lightView);
	for (auto& corner : frustumCorners)
	{
		const vec3 cornerLVspace = vec3(lightView * vec4(corner, 1.0f));
		min = glm::min(min, cornerLVspace);
		max = glm::max(max, cornerLVspace);
	}
	min.x -= ImguiData.OrthoSizePadding.x;
	min.y -= ImguiData.OrthoSizePadding.y;
	max.x += ImguiData.OrthoSizePadding.x;
	max.y += ImguiData.OrthoSizePadding.y;
	min *= vec3(ImguiData.OrthoSizeScale, 1.f);
	max *= vec3(ImguiData.OrthoSizeScale, 1.f);
	ShadowsCamera.OrthoMinBounds = vec2(min);
	ShadowsCamera.OrthoMaxBounds = vec2(max);
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
	uint32_t culled = 0;
	for (auto& surface : DrawContext.Surfaces[EMaterialPass::MainColor])
	{
		if (!shadowsFrustum.IsSphereInFrustum(surface.Bounds, surface.Transform))
		{
			culled++;
			continue;
		}
		// TODO: Frustum cull shadow pass
		// TODO: SRenderObject shadows check
		ShadowsShader.SetUniform(GlUniformLocs::ModelMat, surface.Transform);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, GlBindPoints::Ssbo::VertexBuffer, surface.Buffers.VertexBuffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, surface.Buffers.IndexBuffer);
		if (surface.Buffers.IndexBuffer)
			glDrawElements(surface.Material->PrimitiveType, surface.IndexCount, GL_UNSIGNED_INT, (void*)static_cast<uint64_t>(surface.FirstIndex));
		else
			glDrawArrays(surface.Material->PrimitiveType, surface.FirstIndex, surface.IndexCount);
	}
	std::cout << std::format("shadows culled {} / {}\n", culled, DrawContext.Surfaces[EMaterialPass::MainColor].size());

	// Restore opengl viewport size to window size
	auto& viewport = CEngine::Get()->Viewport;
	glViewport(0, 0, viewport.SizeX, viewport.SizeY);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glUseProgram(0);

}
