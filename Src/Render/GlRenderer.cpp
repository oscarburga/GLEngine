#include <glad/glad.h>
#include "GlRenderer.h"
#include <iostream>
#include <format>
#include "Engine.h"
#include "Math/EngineMath.h"
#include <Assets/AssetLoader.h>

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
	// Nothing for now
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
		SceneData.AmbientColor = glm::vec4(0.1f);
		SceneData.SunlightDirection = glm::vec4(glm::normalize(glm::vec3(-0.2f, -1.f, -0.3f)), 2.f);
		SceneData.SunlightColor = glm::vec4(1.f);
		glNamedBufferStorage(*SceneDataBuffer, sizeof(SSceneData), &SceneData, GL_DYNAMIC_STORAGE_BIT);
		glBindBufferBase(GL_UNIFORM_BUFFER, GlBindPoints::Ubo::SceneData, *SceneDataBuffer);
	}

	CAssetLoader::Create();
	if (auto pvpShader = CAssetLoader::LoadShaderProgram("Shaders/pvpShader.vert", "Shaders/pvpShader_pbr.frag"))
	{
		PvpShader = *pvpShader;
	}
	else assert(false);

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
	
	// Refresh SceneData
	PvpShader.Use();
	ActiveCamera.UpdateSceneData(SceneData);
	glNamedBufferSubData(*SceneDataBuffer, 0, sizeof(SSceneData), &SceneData);
	// Draw main color & masked objects
	for (uint8_t pass = EMaterialPass::MainColor; pass <= EMaterialPass::MainColorMasked; pass++)
	{
		for (auto& surface : MainDrawContext.Surfaces[pass])
		{
			PvpShader.SetUniform(GlUniformLocs::ModelMat, surface.Transform);
			PvpShader.SetUniform(GlUniformLocs::PbrColorTex, GlTexUnits::PbrColor);
			PvpShader.SetUniform(GlUniformLocs::PbrMetalRoughTex, GlTexUnits::PbrMetalRough);
			PvpShader.SetUniform("ignoreLighting", surface.Material->bIgnoreLighting); // this one will be moved to material UBO soon
			glBindTextureUnit(GlTexUnits::PbrColor, *surface.Material->ColorTex.Texture);
			glBindSampler(GlTexUnits::PbrColor, *surface.Material->ColorTex.Sampler);
			glBindTextureUnit(GlTexUnits::PbrMetalRough, *surface.Material->MetalRoughTex.Texture);
			glBindSampler(GlTexUnits::PbrMetalRough, *surface.Material->MetalRoughTex.Sampler);
			glBindBufferBase(GL_UNIFORM_BUFFER, GlBindPoints::Ubo::PbrMaterial, surface.Material->DataBuffer);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, GlBindPoints::Ssbo::VertexBuffer, surface.Buffers.VertexBuffer);
			if (surface.Buffers.IndexBuffer)
			{
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *surface.Buffers.IndexBuffer);
				glDrawElements(surface.Material->PrimitiveType, surface.IndexCount, GL_UNSIGNED_INT, (void*)static_cast<uint64_t>(surface.FirstIndex));
			}
			else
			{
				glDrawArrays(surface.Material->PrimitiveType, surface.FirstIndex, surface.IndexCount);
			}
		}
		MainDrawContext.Surfaces[pass].clear();
	}

	// Draw Blend, unoptimized sort. Could keep a sorted map and only update objects that move
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
			const glm::vec3& loci = blendObjects[i].Transform[3];
			const glm::vec3& locj = blendObjects[j].Transform[3];
			const float di = glm::length2(camPos - loci);
			const float dj = glm::length2(camPos - locj);
			return di > dj;
		});
		for (size_t& idx : indices)
		{
			auto& surface = blendObjects[idx];
			PvpShader.SetUniform(GlUniformLocs::ModelMat, surface.Transform);
			PvpShader.SetUniform(GlUniformLocs::PbrColorTex, GlTexUnits::PbrColor);
			PvpShader.SetUniform(GlUniformLocs::PbrMetalRoughTex, GlTexUnits::PbrMetalRough);
			PvpShader.SetUniform("ignoreLighting", surface.Material->bIgnoreLighting); // this one will be moved to material UBO soon
			glBindTextureUnit(GlTexUnits::PbrColor, *surface.Material->ColorTex.Texture);
			glBindSampler(GlTexUnits::PbrColor, *surface.Material->ColorTex.Sampler);
			glBindTextureUnit(GlTexUnits::PbrMetalRough, *surface.Material->MetalRoughTex.Texture);
			glBindSampler(GlTexUnits::PbrMetalRough, *surface.Material->MetalRoughTex.Sampler);
			glBindBufferBase(GL_UNIFORM_BUFFER, GlBindPoints::Ubo::PbrMaterial, surface.Material->DataBuffer);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, GlBindPoints::Ssbo::VertexBuffer, surface.Buffers.VertexBuffer);
			if (surface.Buffers.IndexBuffer)
			{
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *surface.Buffers.IndexBuffer);
				glDrawElements(surface.Material->PrimitiveType, surface.IndexCount, GL_UNSIGNED_INT, (void*)static_cast<uint64_t>(surface.FirstIndex));
			}
			else
			{
				glDrawArrays(surface.Material->PrimitiveType, surface.FirstIndex, surface.IndexCount);
			}
		}
		blendObjects.clear();
		glDisable(GL_BLEND);
	}
}

void CGlRenderer::OnWindowResize(CEngine* Engine, const SViewport& Viewport)
{
	glViewport(0, 0, Viewport.SizeX, Viewport.SizeY);
}
