#include <glad/glad.h>
#include "GlRenderer.h"
#include <iostream>
#include <format>
#include "Engine.h"
#include "Math/EngineMath.h"
#include <Assets/AssetLoader.h>
#include <imgui.h>

CGlRenderer* CGlRenderer::Renderer = nullptr;
int CGlRenderer::UBOOffsetAlignment = 16;

namespace
{
	/*
	* Shamelessly copied from https ://learnopengl.com/In-Practice/Debugging
	*/ 
	void APIENTRY GlDebugCallback(GLenum source, GLenum type, unsigned int id, GLenum severity, GLsizei length, const char* message, const void* userParam)
	{
		// ignore non-significant error/warning codes
		if (id == 131169 || id == 131185 || id == 131218 || id == 131204) 
			return; 

		std::cerr << std::format("----------------\nDebug message ({}): {}\n", id, message);

		switch (source)
		{
			case GL_DEBUG_SOURCE_API:             std::cerr << "Source: API"; break;
			case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   std::cerr << "Source: Window System"; break;
			case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cerr << "Source: Shader Compiler"; break;
			case GL_DEBUG_SOURCE_THIRD_PARTY:     std::cerr << "Source: Third Party"; break;
			case GL_DEBUG_SOURCE_APPLICATION:     std::cerr << "Source: Application"; break;
			case GL_DEBUG_SOURCE_OTHER:           std::cerr << "Source: Other"; break;
		} 
		std::cerr << '\n';

		switch (type)
		{
			case GL_DEBUG_TYPE_ERROR:               std::cerr << "Type: Error"; break;
			case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cerr << "Type: Deprecated Behaviour"; break;
			case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  std::cerr << "Type: Undefined Behaviour"; break;
			case GL_DEBUG_TYPE_PORTABILITY:         std::cerr << "Type: Portability"; break;
			case GL_DEBUG_TYPE_PERFORMANCE:         std::cerr << "Type: Performance"; break;
			case GL_DEBUG_TYPE_MARKER:              std::cerr << "Type: Marker"; break;
			case GL_DEBUG_TYPE_PUSH_GROUP:          std::cerr << "Type: Push Group"; break;
			case GL_DEBUG_TYPE_POP_GROUP:           std::cerr << "Type: Pop Group"; break;
			case GL_DEBUG_TYPE_OTHER:               std::cerr << "Type: Other"; break;
		} std::cerr << '\n';
		
		switch (severity)
		{
			case GL_DEBUG_SEVERITY_HIGH:         std::cerr << "Severity: high"; break;
			case GL_DEBUG_SEVERITY_MEDIUM:       std::cerr << "Severity: medium"; break;
			case GL_DEBUG_SEVERITY_LOW:          std::cerr << "Severity: low"; break;
			case GL_DEBUG_SEVERITY_NOTIFICATION: std::cerr << "Severity: notification"; break;
		} std::cerr << '\n';
		std::cerr << '\n';
	}
}

CGlRenderer::~CGlRenderer()
{
	glDeleteBuffers(1, &*SceneDataBuffer);
	glDeleteVertexArrays(1, &*EmptyVao);
	CAssetLoader::Destroy();
}

CGlRenderer* CGlRenderer::Create(GlFunctionLoaderFuncType func)
{
	if (!Renderer)
	{
		Renderer = new CGlRenderer();
		Renderer->Init(func);
	}
	return Renderer;
}

void CGlRenderer::Init(GlFunctionLoaderFuncType func)
{
	if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(func)))
	{
		std::cerr << "Failed to init GLAD\n";
		std::abort();
	}
	int flags;
	glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
	if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
	{
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS); 
		glDebugMessageCallback(GlDebugCallback, nullptr);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
		std::cout << "Debug callback activated\n";
	}
	{
		int value;
		glGetIntegerv(GL_MAX_UNIFORM_LOCATIONS, &value);
		std::cout << std::format("Max Uniform Variable Locations : {}\n", value);
		glGetIntegerv(GL_MAX_UNIFORM_BUFFER_BINDINGS, &value);
		std::cout << std::format("Max Uniform Buffer Bindings: {}\n", value);
		glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &value);
		std::cout << std::format("Max Uniform Block Size: {} bytes ({} Mb)\n", value, value / (1'000'000));
		glGetIntegerv(GL_MAX_VERTEX_UNIFORM_BLOCKS, &value);
		std::cout << std::format("Max Uniform Blocks (VERTEX SHADER): {}\n", value);
		glGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_BLOCKS, &value);
		std::cout << std::format("Max Uniform Blocks (FRAGMENT SHADER): {}\n", value);
		glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &value);
		std::cout << std::format("Uniform Block Buffer Offset Alignment: {}\n", value);
		UBOOffsetAlignment = value;
	}
	// Create & bind empty VAO
	{
		glCreateVertexArrays(1, &*EmptyVao);
		glBindVertexArray(*EmptyVao);
	}
	// Scene data uniform buffer
	{
		glCreateBuffers(1, &*SceneDataBuffer);
		SceneData.SunlightDirection = glm::vec4(glm::normalize(glm::vec3(-0.2f, -1.f, -0.3f)), 2.f);
		SceneData.SunlightColor = glm::vec4(1.f);
		ImguiData.SunlightDirection = SceneData.SunlightDirection;
		glNamedBufferStorage(*SceneDataBuffer, sizeof(SSceneData), &SceneData, GL_DYNAMIC_STORAGE_BIT);
		glBindBufferBase(GL_UNIFORM_BUFFER, GlBindPoints::Ubo::SceneData, *SceneDataBuffer);
	}
	// Simple quad2d
	{
		float quadVertices[] = {
			// positions   // texCoords
			-1.0f,  1.0f,  0.0f, 1.0f,
			-1.0f, -1.0f,  0.0f, 0.0f,
			 1.0f, -1.0f,  1.0f, 0.0f,

			-1.0f,  1.0f,  0.0f, 1.0f,
			 1.0f, -1.0f,  1.0f, 0.0f,
			 1.0f,  1.0f,  1.0f, 1.0f
		};	
		glCreateBuffers(1, &*Quad2DBuffer);
		glNamedBufferStorage(*Quad2DBuffer, sizeof(quadVertices), quadVertices, 0);
	}

	CAssetLoader::Create();

	ShadowPass.Init();

	SShaderLoadArgs fsArgs("Shaders/pvpMesh.frag", { { ShadowPass.NumCascadesShaderArgName, std::to_string(ShadowPass.GetNumCascades()) } });
	if (auto pvpShader = CAssetLoader::LoadShaderProgram("Shaders/pvpMesh.vert", fsArgs))
		PvpShader = *pvpShader;

	// TODO: separate simplequad.frag from the shadow depth debug shader.
	if (auto quadShader = CAssetLoader::LoadShaderProgram("Shaders/simplequad.vert", "Shaders/simplequad.frag"))
		QuadShader = *quadShader;

	assert(PvpShader.Id && QuadShader.Id);
}

void CGlRenderer::Destroy()
{
	if (Renderer)
	{
		delete Renderer;
		Renderer = nullptr;
	}
}

void CGlRenderer::RenderScene(float deltaTime)
{
	glEnable(GL_DEPTH_TEST);
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	// TODO: figure out negative determinants for flipping the front face
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	// Refresh SceneData
	SceneData.SunlightDirection = vec4(glm::normalize(vec3(ImguiData.SunlightDirection)), ImguiData.SunlightDirection.w);
	ActiveCamera.UpdateSceneData(SceneData);
	ShadowPass.UpdateSceneData(SceneData, ActiveCamera);
	glNamedBufferSubData(*SceneDataBuffer, 0, sizeof(SSceneData), &SceneData);
	ShadowPass.RenderShadowDepth(SceneData, MainDrawContext);

	// TEMP: will need to fix the debug view of the shadows texture with csm
	if (ImguiData.bShowShadowDepthMap)
	{
		// render shadow depth onto quad to screen
		QuadShader.Use();
		glBindTextureUnit(GlTexUnits::ShadowMap, *ShadowPass.ShadowsTexArray);
		QuadShader.SetUniform(GlUniformLocs::ShadowDepthTexture, GlTexUnits::ShadowMap);
		QuadShader.SetUniform(GlUniformLocs::DebugShadowDepthMapIndex, ImguiData.ShadowDepthMapIndex);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, GlBindPoints::Ssbo::VertexBuffer, Quad2DBuffer);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		// Clear all
		std::for_each(MainDrawContext.Surfaces.begin(), MainDrawContext.Surfaces.end(), [&](auto& vec)
		{
			vec.clear();
		});
		return;
	}

	// Culling... lots of room for optimization here
	SFrustum mainCameraFrustum; 
	ActiveCamera.CalcFrustum(&mainCameraFrustum, nullptr);
	// Draw main color & masked objects
	// PvpShader.SetUniform(GlUniformLocs::ShowDebugNormals, true);

	// Draw to main framebuffer now, and simply toss multisampling on.
	// IF WE EVER CHANGE THE COLOR PASS TO RENDER TO ANOTHER FRAMEBUFFER, ADDITIONAL
	// WORK WILLL NEED TO BE MADE
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glEnable(GL_MULTISAMPLE); 
	PvpShader.Use();
	PvpShader.SetUniform(GlUniformLocs::PbrColorTex, GlTexUnits::PbrColor);
	PvpShader.SetUniform(GlUniformLocs::PbrMetalRoughTex, GlTexUnits::PbrMetalRough);
	PvpShader.SetUniform(GlUniformLocs::NormalTex, GlTexUnits::Normal);
	PvpShader.SetUniform(GlUniformLocs::OcclusionTex, GlTexUnits::PbrOcclusion);
	PvpShader.SetUniform(GlUniformLocs::ShadowDepthTexture, GlTexUnits::ShadowMap);
	PvpShader.SetUniform(GlUniformLocs::DebugCsmTint, ImguiData.bDebugCsmTint);
	glBindTextureUnit(GlTexUnits::ShadowMap, *ShadowPass.ShadowsTexArray);
	ImguiData.CulledNum = ImguiData.TotalNum = 0;

	static const auto RenderObject = [&](SRenderObject& surface)
	{
		if (!mainCameraFrustum.IsSphereInFrustum(surface.Bounds, surface.WorldTransform))
		{
			++ImguiData.CulledNum;
			return;
		}
		PvpShader.SetUniform(GlUniformLocs::ModelMat, surface.RenderTransform);
		PvpShader.SetUniform(GlUniformLocs::DebugIgnoreLighting, surface.Material->bIgnoreLighting); // this one will be moved to material UBO soon
		PvpShader.SetUniform(GlUniformLocs::HasJoints, surface.VertexJointsDataBuffer && surface.JointMatricesBuffer);

		glBindTextureUnit(GlTexUnits::PbrColor, *surface.Material->ColorTex.Texture);
		glBindSampler(GlTexUnits::PbrColor, *surface.Material->ColorTex.Sampler);

		glBindTextureUnit(GlTexUnits::PbrMetalRough, *surface.Material->MetalRoughTex.Texture);
		glBindSampler(GlTexUnits::PbrMetalRough, *surface.Material->MetalRoughTex.Sampler);

		glBindTextureUnit(GlTexUnits::Normal, *surface.Material->NormalTex.Texture);
		glBindSampler(GlTexUnits::Normal, *surface.Material->NormalTex.Sampler);

		glBindTextureUnit(GlTexUnits::PbrOcclusion, *surface.Material->OcclusionTex.Texture);
		glBindSampler(GlTexUnits::PbrOcclusion, *surface.Material->OcclusionTex.Sampler);

		glBindBufferBase(GL_UNIFORM_BUFFER, GlBindPoints::Ubo::PbrMaterial, surface.Material->DataBuffer);
		glBindBufferBase(GL_UNIFORM_BUFFER, GlBindPoints::Ubo::JointMatrices, surface.JointMatricesBuffer);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, GlBindPoints::Ssbo::VertexBuffer, surface.VertexBuffer);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, GlBindPoints::Ssbo::VertexJointBuffer, surface.VertexJointsDataBuffer);
		
		constexpr int windingOrder[] = { GL_CW, GL_CCW };
		// TODO: there's something wrong somewhere with the face culling / winding order, should not need to invert this...
		// Figure it out at some point
		glFrontFace(windingOrder[!surface.bIsCCW]);

		if (surface.IndexBuffer)
		{
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *surface.IndexBuffer);
			glDrawElements(surface.Material->PrimitiveType, surface.IndexCount, GL_UNSIGNED_INT, (void*)static_cast<uint64_t>(surface.FirstIndex));
		}
		else
		{
			glDrawArrays(surface.Material->PrimitiveType, surface.FirstIndex, surface.IndexCount);
		}
	};

	for (uint8_t pass = EMaterialPass::MainColor; pass <= EMaterialPass::MainColorMasked; pass++)
	{
		ImguiData.TotalNum += (uint32_t)MainDrawContext.Surfaces[pass].size();
		for (auto& surface : MainDrawContext.Surfaces[pass])
			RenderObject(surface);

		MainDrawContext.Surfaces[pass].clear();
	}

	// Basic blend, no OIT
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		auto& blendObjects = MainDrawContext.Surfaces[EMaterialPass::Transparent];
		auto& indices = MainDrawContext.BlendIndices;
		indices.resize(blendObjects.size());
		std::iota(indices.begin(), indices.end(), 0); 
		std::sort(indices.begin(), indices.end(), [&, camPos = glm::vec3(SceneData.CameraPos)](size_t& i, size_t& j)
		{
			const glm::vec3& loci = blendObjects[i].RenderTransform[3];
			const glm::vec3& locj = blendObjects[j].RenderTransform[3];
			const float di = glm::length2(camPos - loci);
			const float dj = glm::length2(camPos - locj);
			return di > dj;
		});

		ImguiData.TotalNum += (uint32_t)MainDrawContext.Surfaces[EMaterialPass::Transparent].size();
		for (size_t& idx : indices)
		{
			auto& surface = blendObjects[idx];
			RenderObject(surface);
		}
		blendObjects.clear();
		glDisable(GL_BLEND);
	}

	glDisable(GL_MULTISAMPLE);
	// std::cout << std::format("Culled objects {} / {}\n", culledObjects, totalObjects);
}

void CGlRenderer::OnWindowResize(CEngine* Engine, const SViewport& Viewport)
{
	glViewport(0, 0, Viewport.SizeX, Viewport.SizeY);
}

void CGlRenderer::ShowImguiPanel()
{
	if (ImGui::Begin("Renderer", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text("Shadows culling %u/%u", ShadowPass.ImguiData.CulledNum, ShadowPass.ImguiData.TotalNum);
		ImGui::Text("Color pass culling %u/%u", ImguiData.CulledNum, ImguiData.TotalNum);

		if (ImGui::CollapsingHeader("Debug"))
		{
			ImGui::Checkbox("Show shadow map", &ImguiData.bShowShadowDepthMap);
			ImGui::Checkbox("Show csm tint", &ImguiData.bDebugCsmTint);
			ImGui::InputInt("Shadow map index", &ImguiData.ShadowDepthMapIndex);
		}

		if (ImGui::CollapsingHeader("Scene settings", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::InputFloat("Camera FOV", &ActiveCamera.PerspectiveFOV);
			ImGui::InputFloat4("Sunlight dir (w is intensity)", &ImguiData.SunlightDirection.x);
			ImGui::ColorEdit3("Sunlight color", &SceneData.SunlightColor.x);
			ImGui::InputFloat2("Shadows ortho size scale", &ShadowPass.ImguiData.OrthoSizeScale.x);
			ImGui::InputFloat2("Shadows ortho size pad", &ShadowPass.ImguiData.OrthoSizePadding.x);
		}
	}
	ImGui::End();
}
