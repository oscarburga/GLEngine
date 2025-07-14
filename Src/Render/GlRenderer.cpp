#include <glad/glad.h>
#include "GlRenderer.h"
#include <iostream>
#include <format>
#include "Engine.h"
#include "Math/EngineMath.h"
#include <Assets/AssetLoader.h>
#include <imgui.h>

CGlRenderer* CGlRenderer::Renderer = nullptr;

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
	// Log some useful values
	{
		int value;
		glGetIntegerv(GL_MAX_ELEMENTS_INDICES, &value);
		std::cout << std::format("Max elements indices: {}\n", value);
		glGetIntegerv(GL_MAX_ELEMENTS_VERTICES, &value);
		std::cout << std::format("Max Elements vertices : {}\n", value);
		glGetIntegerv(GL_MAX_UNIFORM_LOCATIONS, &value);
		std::cout << std::format("Max Uniform Variable Locations : {}\n", value);
		glGetIntegerv(GL_MAX_UNIFORM_BUFFER_BINDINGS, &value);
		std::cout << std::format("Max Uniform Buffer Bindings: {}\n", value);
		glGetIntegerv(GL_MAX_VERTEX_UNIFORM_BLOCKS, &value);
		std::cout << std::format("Max Uniform Blocks (VERTEX SHADER): {}\n", value);
		glGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_BLOCKS, &value);
		std::cout << std::format("Max Uniform Blocks (FRAGMENT SHADER): {}\n", value);
		glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &value);
		std::cout << std::format("Max Fragment Texture Image Units: {}\n", value);
		glGetIntegerv(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS, &value);
		std::cout << std::format("Max Vertex Texture Image Units: {}\n", value);
		glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &value);
		std::cout << std::format("Max Combined Texture Image Units: {}\n", value);
		glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &value);
		std::cout << std::format("Uniform Block Buffer Offset Alignment: {}\n", value);
		UBOOffsetAlignment = value;
		glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &value);
		std::cout << std::format("Max Uniform Block Size: {} bytes ({} Mb)\n", value, float(value) / (1'000'000));
		UBOMaxBlockSize = value;
		glGetIntegerv(GL_MAX_SHADER_STORAGE_BLOCK_SIZE, &value);
		std::cout << std::format("Max SSBO size: {} bytes ({} Mb)\n", value, value / (1'000'000));
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
	// Main buffers
	{
		// TODO: Set all these values from some config file (maybe this is where I can add some 
		// LUA scripting for funzies? :D 
		constexpr GLsizeiptr MainBufferSize = 1 << 30; // 1 GiB (1073 ish MB)
		constexpr GLsizeiptr BonesBufferSize = 1 << 28; // about 268 MB
		ShaderMaxMaterialSize = UBOMaxBlockSize / sizeof(SPbrMaterial);
		DrawDataBufferMaxSize = UBOMaxBlockSize / sizeof(SDrawObjectGpuData);

		MainVertexBuffer = SGlBufferVector(MainBufferSize);
		MainIndexBuffer = SGlBufferVector(MainBufferSize);
		MainBonesBuffer = SGlBufferVector(BonesBufferSize);
		MainMaterialBuffer = SGlBufferVector(UBOMaxBlockSize);
		JointMatricesBuffer = SGlBufferVector(UBOMaxBlockSize);
		DrawDataBuffer = SGlBufferVector(DrawDataBufferMaxSize * sizeof(SDrawObjectGpuData));
		// TODO: my gpu allows 4mb UBO block size, but spec only guarantees 16kb, which would only be a couple hundred materials / matrices.
		// Add some fallback to use SSBO for materials instead of UBO
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
		// DO NOT put this in the main mesh buffer, its different alignment from SVertex would kapoot the indexing!!
		glCreateBuffers(1, &*Quad2DBuffer);
		glNamedBufferStorage(*Quad2DBuffer, sizeof(quadVertices), quadVertices, 0);
	}

	CAssetLoader::Create();

	ShadowPass.Init();

	SShaderLoadArgs fsArgs("Shaders/pvpMesh.frag");
	fsArgs
		.SetArg(ShadowPass.NumCascadesShaderArgName, ShadowPass.GetNumCascades())
		.SetArg("MAX_MATERIALS", ShaderMaxMaterialSize);
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
	// Refresh SceneData
	SceneData.SunlightDirection = vec4(glm::normalize(vec3(ImguiData.SunlightDirection)), ImguiData.SunlightDirection.w);
	ActiveCamera.UpdateSceneData(SceneData);
	ShadowPass.UpdateSceneData(SceneData, ActiveCamera);
	glNamedBufferSubData(*SceneDataBuffer, 0, sizeof(SSceneData), &SceneData);
	ShadowPass.RenderShadowDepth(SceneData, MainDrawContext);

	glEnable(GL_DEPTH_TEST);
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	// TODO: figure out negative determinants for flipping the front face
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

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
		std::for_each(MainDrawContext.RenderObjects.begin(), MainDrawContext.RenderObjects.end(), [&](auto& vec)
		{
			vec.ClearAll();
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
	PvpShader.SetUniform(GlUniformLocs::PbrOcclusionTex, GlTexUnits::PbrOcclusion);
	PvpShader.SetUniform(GlUniformLocs::ShadowDepthTexture, GlTexUnits::ShadowMap);
	PvpShader.SetUniform(GlUniformLocs::DebugCsmTint, ImguiData.bDebugCsmTint);

	glBindTextureUnit(GlTexUnits::ShadowMap, *ShadowPass.ShadowsTexArray);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, MainIndexBuffer.Id);
	glBindBufferBase(GL_UNIFORM_BUFFER, GlBindPoints::Ubo::PbrMaterial, MainMaterialBuffer.Id);
	glBindBufferBase(GL_UNIFORM_BUFFER, GlBindPoints::Ubo::JointMatrices, JointMatricesBuffer.Id);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, GlBindPoints::Ssbo::VertexBuffer, MainVertexBuffer.Id);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, GlBindPoints::Ssbo::VertexJointBuffer, MainBonesBuffer.Id);

	ImguiData.CulledNum = ImguiData.TotalNum = 0;

	static const auto RenderObject = [&](SRenderObject& surface)
	{
		if (!mainCameraFrustum.IsSphereInFrustum(surface.Bounds, surface.WorldTransform))
		{
			++ImguiData.CulledNum;
			return;
		}

		// PER-OBJECT UNIFORMS
		// Eventually we move this & other per-object data into big buffer(s) with per-object data
		{
			PvpShader.SetUniform(GlUniformLocs::ModelMat, surface.RenderTransform);
			PvpShader.SetUniform(GlUniformLocs::HasJoints, surface.VertexJointsDataBuffer && surface.JointMatricesBuffer);
			PvpShader.SetUniform(GlUniformLocs::JointMatricesBaseIndex, (int)surface.JointMatricesBuffer.GetHeadInElems());
			if (surface.VertexJointsDataBuffer)
			{
				// To get the index offset in joints data buffer, we substract the base vertex to bring the idx back to [0:N) and add the joints base index for the bone buffer
				int64_t boneBufferOffset = int64_t(surface.VertexJointsDataBuffer.GetHeadInElems()) - int64_t(surface.VertexBuffer.GetHeadInElems());
				assert(boneBufferOffset <= std::numeric_limits<int>::max());
				PvpShader.SetUniform(GlUniformLocs::BonesIndexOffset, (int)boneBufferOffset);
			}
			PvpShader.SetUniform(GlUniformLocs::MaterialIndex, (int)surface.Material->DataBuffer.GetHeadInElems());
		}

		glBindTextureUnit(GlTexUnits::PbrColor, *surface.Material->ColorTex.Texture);
		glBindSampler(GlTexUnits::PbrColor, *surface.Material->ColorTex.Sampler);

		glBindTextureUnit(GlTexUnits::PbrMetalRough, *surface.Material->MetalRoughTex.Texture);
		glBindSampler(GlTexUnits::PbrMetalRough, *surface.Material->MetalRoughTex.Sampler);

		glBindTextureUnit(GlTexUnits::Normal, *surface.Material->NormalTex.Texture);
		glBindSampler(GlTexUnits::Normal, *surface.Material->NormalTex.Sampler);

		glBindTextureUnit(GlTexUnits::PbrOcclusion, *surface.Material->OcclusionTex.Texture);
		glBindSampler(GlTexUnits::PbrOcclusion, *surface.Material->OcclusionTex.Sampler);

		constexpr int windingOrder[] = { GL_CW, GL_CCW };
		glFrontFace(windingOrder[surface.bIsCCW]);

		// TODO abstract these draw calls into a function, this code is repeated in shadow depth pass.
		if (surface.IndexBuffer)
		{
			void* offset = (void*)(surface.IndexBuffer.Head + (surface.FirstIndex * sizeof(GLuint))); // offset is in BYTESSSS not in index type!!
			GLsizei indexCount = surface.IndexCount;
			GLint baseVertex = (GLint)surface.VertexBuffer.GetHeadInElems();
			glMultiDrawElementsBaseVertex(
				surface.Material->PrimitiveType,
				&indexCount,
				GL_UNSIGNED_INT,
				&offset, // offset is in BYTESSSS not in index type!!
				1,
				&baseVertex // add to each index so it goes to the correct slice of the big ssbo
			);
		}
		else
		{
			GLint first = surface.FirstIndex + GLint(surface.VertexBuffer.GetHeadInElems());
			GLsizei count = surface.IndexCount;
			glMultiDrawArrays(
				surface.Material->PrimitiveType,
				&first,
				&count,
				1
			);
		}
	};

	for (uint8_t pass = EMaterialPass::MainColor; pass <= EMaterialPass::MainColorMasked; pass++)
	{
		ImguiData.TotalNum += (uint32_t)MainDrawContext.RenderObjects[pass].TotalSize;
		SRenderObjectContainer& renderObjects = MainDrawContext.RenderObjects[pass];
		// TODO batch into calls, but that will have to wait until bindless textures
		for (int CCW = 0; CCW < 2; CCW++)
		{
			for (int indexed = 0; indexed < 2; indexed++)
			{
				for (SRenderObject& renderObject : renderObjects.TriangleObjects[CCW][indexed])
				{
					RenderObject(renderObject);
				}
			}
		}

		MainDrawContext.RenderObjects[pass].ClearAll();
	}

	// Basic blend, no OIT
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		auto& blendObjects = MainDrawContext.RenderObjects[EMaterialPass::Transparent];
		auto& indices = BlendIndices;

		ImguiData.TotalNum += (uint32_t)blendObjects.TotalSize;

		std::vector<SRenderObject>& renderObjects = blendObjects.OtherObjects;
		indices.resize(renderObjects.size());
		std::iota(indices.begin(), indices.end(), 0);
		std::sort(indices.begin(), indices.end(), [&, camPos = glm::vec3(SceneData.CameraPos)](uint32_t i, uint32_t j)
		{
			const glm::vec3& loci = renderObjects[i].RenderTransform[3];
			const glm::vec3& locj = renderObjects[j].RenderTransform[3];
			const float di = glm::length2(camPos - loci);
			const float dj = glm::length2(camPos - locj);
			return di > dj;
		});

		for (uint32_t idx : indices)
		{
			auto& surface = renderObjects[idx];
			RenderObject(surface);
		}
		blendObjects.ClearAll();
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
