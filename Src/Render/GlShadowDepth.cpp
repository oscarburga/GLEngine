#include <glad/glad.h>
#include "Engine.h"
#include "Math/EngineMath.h"
#include "GlShadowDepth.h"
#include "Assets/AssetLoader.h"
#include <Utils/Defer.h>

CGlShadowDepthPass::~CGlShadowDepthPass()
{
}

void CGlShadowDepthPass::Init(uint32_t width, uint32_t height)
{
	Width = width;
	Height = height;
	glCreateFramebuffers(1, &*ShadowsFbo);
	glCreateTextures(GL_TEXTURE_2D, 1, &*ShadowsTexture);
	glTextureStorage2D(*ShadowsTexture, 1, GL_DEPTH_COMPONENT32F, width, height);
	glTextureParameteri(*ShadowsTexture, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTextureParameteri(*ShadowsTexture, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTextureParameteri(*ShadowsTexture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTextureParameteri(*ShadowsTexture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	glNamedFramebufferTexture(*ShadowsFbo, GL_DEPTH_ATTACHMENT, *ShadowsTexture, 0);
	glNamedFramebufferDrawBuffer(*ShadowsFbo, GL_NONE);
	glNamedFramebufferReadBuffer(*ShadowsFbo, GL_NONE);

	auto shader = CAssetLoader::LoadShaderProgram("pvpShadows.vert", "empty.frag");
	assert(shader);
	ShadowsShader = *shader;
}

void CGlShadowDepthPass::UpdateSceneData(SSceneData& SceneData)
{
	float shadowsNear = 0.1f, shadowsFar = 7.5f;
	glm::mat4 lightProj = glm::orthoLH(-10.f, 10.f, -10.f, 10.f, shadowsNear, shadowsFar);
	glm::mat4 lightView = glm::lookAtLH(glm::vec3(-2.f, 10.f, -1.f), glm::vec3(0.0f), World::Up);
	SceneData.LightSpaceTransform = lightProj * lightView;
}

void CGlShadowDepthPass::RenderShadowDepth(SSceneData& SceneData, SDrawContext& DrawContext)
{
	Defer cleanup([&]
	{
		// Restore opengl viewport size to window size
		auto& viewport = CEngine::Get()->Viewport;
		glViewport(0, 0, viewport.SizeX, viewport.SizeY);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glUseProgram(0);
	});
	glViewport(0, 0, Width, Height);
	glBindFramebuffer(GL_FRAMEBUFFER, *ShadowsFbo);
	glClear(GL_DEPTH_BUFFER_BIT);

	ShadowsShader.Use();
	for (auto& surface : DrawContext.Surfaces[EMaterialPass::MainColor])
	{
		// TODO: Frustum cull shadow pass
		// TODO: SRenderObject shadows check
		ShadowsShader.SetUniform(GlUniformLocs::ModelMat, surface.Transform);
		if (surface.Buffers.IndexBuffer)
			glDrawElements(surface.Material->PrimitiveType, surface.IndexCount, GL_UNSIGNED_INT, (void*)static_cast<uint64_t>(surface.FirstIndex));

		glDrawArrays(surface.Material->PrimitiveType, surface.FirstIndex, surface.IndexCount);
	}
}
