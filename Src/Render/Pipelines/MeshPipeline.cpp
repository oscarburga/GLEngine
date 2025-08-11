#include "MeshPipeline.h"

#include "glad/glad.h"

#include "Assets/AssetLoader.h"
#include "Math/Frustum.h"
#include "Render/GlDrawCommands.h"
#include "Render/GlCamera.h"
#include "Render/GlRenderer.h"
#include "Render/Pipelines/CsmPipeline.h"
#include "Render/RenderObject.h"

void CMeshPipeline::Init()
{

	DrawCommands = std::make_unique<SDrawCommands>(CGlRenderer::DrawDataBufferMaxSize);
	SShaderLoadArgs vsArgs("Shaders/pvpMeshMdi.vert");
	vsArgs
		.SetArg(CGlRenderer::NumCascadesShaderArgName, CGlRenderer::NumCascades)
		.SetArg("MAX_DRAWS", CGlRenderer::DrawDataBufferMaxSize)
		.SetArg("MAX_MATERIALS", CGlRenderer::ShaderMaxMaterialSize);

	SShaderLoadArgs fsArgs("Shaders/pvpMeshMdi.frag");
	fsArgs
		.SetArg(CGlRenderer::NumCascadesShaderArgName, CGlRenderer::NumCascades)
		.SetArg("MAX_DRAWS", CGlRenderer::DrawDataBufferMaxSize)
		.SetArg("MAX_MATERIALS", CGlRenderer::ShaderMaxMaterialSize);

	if (auto pvpShader = CAssetLoader::LoadShaderProgram(vsArgs, fsArgs))
		PvpShader = *pvpShader;

}

void CMeshPipeline::UpdateSceneData(SSceneData& SceneData, const SGlCamera& Camera)
{
	// Nothing for now.
}

void CMeshPipeline::PrepassDrawDataBuffer(CGlRenderer& renderer)
{
	SFrustum mainCameraFrustum; 
	renderer.ActiveCamera->CalcFrustum(&mainCameraFrustum, nullptr);

	DrawCommands->ResetBuffers();
	for (uint8_t pass = EMaterialPass::MainColor; pass <= EMaterialPass::MainColorMasked; ++pass)
	{
		const SRenderObjectContainer& renderObjects = renderer.MainDrawContext->RenderObjects[pass];
		renderer.ImguiData.TotalNum += (uint32_t)renderObjects.TotalSize;
		renderer.ImguiData.CulledNum += DrawCommands->PopulateBuffers(renderObjects, false, [&](const SRenderObject& surface) -> bool
		{
			return !mainCameraFrustum.IsSphereInFrustum(surface.Bounds, surface.WorldTransform);
		});
	}
}

void CMeshPipeline::Render(const CGlRenderer& renderer)
{
	glEnable(GL_DEPTH_TEST);
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	// TODO: figure out negative determinants for flipping the front face
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glEnable(GL_MULTISAMPLE); 
	PvpShader.Use();
	PvpShader.SetUniform(GlUniformLocs::ShadowDepthTexture, GlTexUnits::ShadowMap);
	PvpShader.SetUniform(GlUniformLocs::DebugCsmTint, renderer.ImguiData.bDebugCsmTint);

	glBindTextureUnit(GlTexUnits::ShadowMap, *renderer.CsmPipeline->ShadowsTexArray);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderer.MainIndexBuffer.Id);
	glBindBuffer(GL_DRAW_INDIRECT_BUFFER, DrawCommands->MdiBuffer.Id);
	glBindBufferBase(GL_UNIFORM_BUFFER, GlBindPoints::Ubo::JointMatrices, renderer.JointMatricesBuffer.Id);
	glBindBufferBase(GL_UNIFORM_BUFFER, GlBindPoints::Ubo::DrawData, DrawCommands->DrawDataBuffer.Id);
	glBindBufferBase(GL_UNIFORM_BUFFER, GlBindPoints::Ubo::PbrMaterial, renderer.MainMaterialBuffer.Id);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, GlBindPoints::Ssbo::VertexBuffer, renderer.MainVertexBuffer.Id);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, GlBindPoints::Ssbo::VertexJointBuffer, renderer.MainBonesBuffer.Id);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, GlBindPoints::Ssbo::TextureBuffers, renderer.TextureHandlesBuffer.Id);

	// Draw indirect color passes
	for (int CCW = 0; CCW < 2; CCW++)
	{
		constexpr int windingOrder[] = { GL_CW, GL_CCW }; 
		glFrontFace(windingOrder[CCW]); // culling backface, so also need to flip this

		if (const std::vector<SGlBufferRangeId>& rangeIds = DrawCommands->GetMdiBufferRanges(CCW); !rangeIds.empty())
		{
			for (const SGlBufferRangeId& rangeId : rangeIds)
			{
				PvpShader.SetUniform(GlUniformLocs::BaseDrawId, (int)rangeId.GetHeadInElems());
				glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, (void*)rangeId.Head, (GLsizei)rangeId.GetNumElems(), 0);
			}
		}
	}
	glDisable(GL_MULTISAMPLE);
}

