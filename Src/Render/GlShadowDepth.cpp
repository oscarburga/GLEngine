#include <glad/glad.h>
#include "Engine.h"
#include "Math/EngineMath.h"
#include "GlShadowDepth.h"
#include "Assets/AssetLoader.h"
#include "GlRenderer.h"

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
	ShadowsCamera.bIsPerspective = true;
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
	glm::vec3 mainCameraFront = glm::rotateByQuat(World::Front, Camera.Rotation);
	SFrustum camFrustum;
	std::array<vec3, 8> frustumCorners;
	Camera.CalcFrustum(&camFrustum, &frustumCorners);
	const vec3 frustumCenter = std::accumulate(frustumCorners.begin(), frustumCorners.end(), vec3{0.0f}) / 8.f;

	glm::vec3 min(std::numeric_limits<float>::max()), max(std::numeric_limits<float>::min());
	const glm::mat4 lightView = glm::lookAt(frustumCenter - vec3(SceneData.SunlightDirection), frustumCenter, World::Up);
	for (auto& corner : frustumCorners)
	{
		const glm::vec3 cornerLVspace = vec3(lightView * vec4(corner, 1.0f));
		min = glm::min(min, cornerLVspace);
		max = glm::max(max, cornerLVspace);
	}
	glm::mat4 lightProj = glm::ortho(min.x, max.x, min.y, max.y, min.z, max.z);
	SceneData.LightSpaceTransform = lightProj * lightView;
}

void CGlShadowDepthPass::RenderShadowDepth(const SSceneData& SceneData, const SDrawContext& DrawContext)
{
	glViewport(0, 0, Width, Height);
	glBindFramebuffer(GL_FRAMEBUFFER, *ShadowsFbo);
	glClear(GL_DEPTH_BUFFER_BIT);

	ShadowsShader.Use();
	for (auto& surface : DrawContext.Surfaces[EMaterialPass::MainColor])
	{
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

	// Restore opengl viewport size to window size
	auto& viewport = CEngine::Get()->Viewport;
	glViewport(0, 0, viewport.SizeX, viewport.SizeY);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glUseProgram(0);

}
