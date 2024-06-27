#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <format>
#include "Graphics/Shader.h"
#include "Math/EngineMath.h"
#include "Assets/AssetLoader.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "Utils/RefIgnore.h"
#include "Engine.h"
#include "Camera.h"
#include "WorldContainers.h"
#include <Utils/Defer.h>

int main(int argc, char** argv)
{
	CEngine* Engine = CEngine::Create();
	float vertices[] = {
		// positions          // normals           // texture coords
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
		 0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,

		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f,

		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
		-0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

		 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,

		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f
	};

	const vec3 cubePositions[] = {
		vec3(-2.4f, 0.4f, 3.5f),
		vec3(2.0f,  5.0f, 15.0f),
		vec3(-1.5f, -2.2f, 2.5f),
		vec3(-3.8f, -2.0f, 12.3f),
		vec3(2.4f, -0.4f, 3.5f),
		vec3(-1.7f,  3.0f, 7.5f),
		vec3(1.3f, -2.0f, 2.5f),
		vec3(1.5f,  2.0f, 2.5f),
		vec3(1.5f,  0.2f, 1.5f),
		vec3(-1.3f,  1.0f, 1.5f)
	};

	std::vector<SVertex> cubeData;
	constexpr int numRows = sizeof(vertices) / (8 * sizeof(float));
	for (int i = 0; i < numRows; i++)
	{
		const int idx = i * 8;
		SVertex v {
			.Position = { vertices[idx], vertices[idx + 1], vertices[idx + 2] },
			.uv_x = vertices[idx + 6],
			.Normal = { vertices[idx + 3], vertices[idx + 4], vertices[idx + 5] },
			.uv_y = vertices[idx + 7],
		};
		v.Color = vec4(v.Normal, 1.f);
		cubeData.emplace_back(v);
	}
	SMeshAsset mesh;
	mesh.Surfaces.emplace_back(SGeoSurface{
		.StartIndex = 0,
		.Count = (uint32_t)cubeData.size(),
	});

	mesh.MeshBuffers.IndexBuffer = GL_NONE;
	GLuint emptyVAO;
	glCreateVertexArrays(1, &emptyVAO);
	glBindVertexArray(emptyVAO);
	glCreateBuffers(1, &mesh.MeshBuffers.VertexBuffer);
	glNamedBufferStorage(mesh.MeshBuffers.VertexBuffer, cubeData.size() * sizeof(SVertex), cubeData.data(), GL_DYNAMIC_STORAGE_BIT);
	auto deferredDelete = Defer([&]()
	{
		glDeleteBuffers(1, &mesh.MeshBuffers.VertexBuffer);
	});

	GCamera& camera = WorldContainers::InputProcessors.emplace_back(GCamera());
	GWorldObject cube;
	cube.Transform.SetPosition(vec3(0.0f, 0.0f, 3.f));
	
	auto pvpShader = CAssetLoader::LoadShaderProgram("Shaders/pvpShader.vert", "Shaders/pvpShader.frag");
	if (!pvpShader)
		std::abort();

	auto basicMeshes = CAssetLoader::LoadGLTFMeshes("GLTF/basicmesh.glb");
	if (!basicMeshes)
		std::abort();

	pvpShader->Use();
	glEnable(GL_DEPTH_TEST);
	Engine->RenderFunc = [&](float deltaTime)
	{
		pvpShader->SetUniform("localToWorld", cube.Transform.GetMatrix());
		pvpShader->SetUniform("worldToCamera", camera.UpdateAndGetViewMatrix());
		pvpShader->SetUniform("cameraToPerspective", camera.GetProjectionMatrix());

		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		constexpr int idx = 2;
		auto& mesh = basicMeshes->at(idx);
		{
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, mesh.MeshBuffers.VertexBuffer);
			if (mesh.MeshBuffers.IndexBuffer != GL_NONE)
			{
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.MeshBuffers.IndexBuffer);
				for (auto& surface : mesh.Surfaces)
					glDrawElements(GL_TRIANGLES, surface.Count, GL_UNSIGNED_INT, (void*)surface.StartIndex);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
			}
			else
			{
				for (auto& surface : mesh.Surfaces)
					glDrawArrays(GL_TRIANGLES, surface.StartIndex, surface.Count);
			}
		}

		// cube
		{
			return;
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, mesh.MeshBuffers.VertexBuffer);
			for (SGeoSurface const& surface : mesh.Surfaces)
			{
				if (mesh.MeshBuffers.IndexBuffer != GL_NONE)
				{
					glDrawElements(GL_TRIANGLES, surface.Count, GL_UNSIGNED_INT, (void*)surface.StartIndex);
				}
				else
				{
					glDrawArrays(GL_TRIANGLES, surface.StartIndex, surface.Count);
				}
			}
		}
	};
	Engine->MainLoop();
	return 0;
}

#if 0
int oldmain(int argc, char** argv)
{
	CEngine* Engine = CEngine::Create();

	float vertices[] = {
		// positions          // normals           // texture coords
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
		 0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,

		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f,

		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
		-0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

		 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,

		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f
	};

	const vec3 cubePositions[] = {
		vec3(-2.4f, 0.4f, 3.5f),
		vec3(2.0f,  5.0f, 15.0f),
		vec3(-1.5f, -2.2f, 2.5f),
		vec3(-3.8f, -2.0f, 12.3f),
		vec3(2.4f, -0.4f, 3.5f),
		vec3(-1.7f,  3.0f, 7.5f),
		vec3(1.3f, -2.0f, 2.5f),
		vec3(1.5f,  2.0f, 2.5f),
		vec3(1.5f,  0.2f, 1.5f),
		vec3(-1.3f,  1.0f, 1.5f)
	};

	GLuint vaos[2];
	glCreateVertexArrays(2, vaos);
	auto [cubeVao, lightVao] = vaos;

	GLuint cubeBuffer;
	glCreateBuffers(1, &cubeBuffer);

	glNamedBufferData(cubeBuffer, sizeof(vertices), vertices, GL_STATIC_DRAW);
	// glVertexArrayElementBuffer(VAO, IBO); // Set the VAO to use IndexBufferObject for the element indices.

	// What buffers it uses in each slot for vertex data
	const int vboBindIndex = 0;// , texCoordsBindIndex = 1;
	// Light cube only uses the veritces, normals dont matter
	glVertexArrayVertexBuffer(lightVao, vboBindIndex, cubeBuffer, 0, 8 * sizeof(float));
	glEnableVertexArrayAttrib(lightVao, 0);
	glVertexArrayAttribFormat(lightVao, 0, 3, GL_FLOAT, GL_FALSE, 0);
	glVertexArrayAttribBinding(lightVao, 0, vboBindIndex);

	glVertexArrayVertexBuffer(cubeVao, vboBindIndex, cubeBuffer, 0, 8 * sizeof(float));

	glEnableVertexArrayAttrib(cubeVao, 0); // enable VAO's array attrib 0
	glVertexArrayAttribFormat(cubeVao, 0, 3, GL_FLOAT, GL_FALSE, 0); // VAO's array attrib 0 has 3 floats,
	glVertexArrayAttribBinding(cubeVao, 0, vboBindIndex); // VAO's array attrib 0 reads from the buffer bound at vboBindIndex (0)

	glEnableVertexArrayAttrib(cubeVao, 1);
	glVertexArrayAttribFormat(cubeVao, 1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float));
	glVertexArrayAttribBinding(cubeVao, 1, vboBindIndex);

	glEnableVertexArrayAttrib(cubeVao, 2);
	glVertexArrayAttribFormat(cubeVao, 2, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float));
	glVertexArrayAttribBinding(cubeVao, 2, vboBindIndex);

	// Textures
	constexpr int numTextures = 4;
	GLuint textures[numTextures];
	constexpr const char* paths[numTextures] = { "Textures/container2.png", "Textures/container2_specular.png", "Textures/container.jpg", "Textures/awesomeface.png" };
	constexpr GLenum inputFormat[numTextures] = { GL_RGBA, GL_RGBA, GL_RGB, GL_RGBA };
	constexpr GLenum textureWrap[numTextures] = { GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_BORDER, GL_CLAMP_TO_BORDER };
	glCreateTextures(GL_TEXTURE_2D, numTextures, textures);
	stbi_set_flip_vertically_on_load(true);
	for (int i = 0; i < numTextures; i++)
	{
		int w, h, c;
		std::filesystem::path p = CAssetLoader::ContentRoot / paths[i];
		if (stbi_uc* texData = stbi_load(p.string().c_str(), &w, &h, &c, 0))
		{
			GLuint texture = textures[i];
			const int numLevels = 1 + (int)floor(log2(std::max(w, h))); // TODO use count leading zero to calculate num mipmaps
			glTextureParameteri(texture, GL_TEXTURE_WRAP_S, textureWrap[i]);
			glTextureParameteri(texture, GL_TEXTURE_WRAP_T, textureWrap[i]);
			float texBorderColor[] = { 0.0f, 1.1f, 0.08f, 1.0f };
			glTextureParameterfv(texture, GL_TEXTURE_BORDER_COLOR, texBorderColor);
			glTextureParameteri(texture, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTextureParameteri(texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // Take closest mipmap and linearly interpolate
			glTextureStorage2D(texture, numLevels, GL_RGB8, w, h);
			glTextureSubImage2D(texture, 0, 0, 0, w, h, inputFormat[i], GL_UNSIGNED_BYTE, texData);
			glGenerateTextureMipmap(texture);
			stbi_image_free(texData);
		}
		else
		{
			std::cerr << "failed to load texture " << p << "\n";
			std::abort();
		}
	}
	auto [crateSpecular, crateDiffuse, woodTexture, faceTexture] = textures;
	
	auto shader = *CAssetLoader::LoadShaderProgram("Shaders/shaderTexCoords.vert", "Shaders/lightmapShader.frag");
	shader.Use();
	// glBindVertexArray(VAO);
	// glBindTextureUnit(0, woodTexture);
	// glBindTextureUnit(1, faceTexture);
	// shader.SetUniform("woodTexture", int(0));
	// shader.SetUniform("faceTexture", int(1));

	auto lightShader = *CAssetLoader::LoadShaderProgram("Shaders/lightningShader.vert", "Shaders/lightningShader.frag");
	lightShader.Use();
	// glPolygonMode(GL_BACK, GL_LINE);
	glEnable(GL_DEPTH_TEST);
	float lastFrameTime = (float)glfwGetTime();
	GCamera& camera = WorldContainers::InputProcessors.emplace_back<GCamera>();
	std::vector<GWorldObject> cubes(10);
	for (size_t i = 0; i<cubes.size(); i++)
	{
		cubes[i].Transform.SetPosition(cubePositions[i]);
	}

	vec3 pointLightPositions[] = {
		vec3( 0.7f,  0.2f,  2.0f),
		vec3( 2.3f, -3.3f, -4.0f),
		vec3(-4.0f,  2.0f, -12.0f),
		vec3( 0.0f,  0.0f, -3.0f)
	};  
	std::vector<GWorldObject> lights(4);
	std::for_each(lights.begin(), lights.end(), [&](auto& obj)
	{
		int i = &obj - lights.data();
		obj.Transform.SetPosition(pointLightPositions[i]);
		obj.Transform.SetScale(vec3(0.2f));
	});

	std::vector<SVertex> cubeData;
	constexpr int numRows = sizeof(vertices) / (8 * sizeof(float));
	for (int i = 0; i < numRows; i++)
	{
		const int idx = i * 8;
		SVertex v {
			.Position = { vertices[idx], vertices[idx + 1], vertices[idx + 2] },
			.uv_x = vertices[idx + 6],
			.Normal = { vertices[idx + 3], vertices[idx + 4], vertices[idx + 5] },
			.uv_y = vertices[idx + 7],
		};
		v.Color = vec4(v.Normal, 1.f);
		cubeData.emplace_back(v);
	}
	SMeshAsset mesh;
	mesh.Surfaces.emplace_back(SGeoSurface{
		.StartIndex = 0,
		.Count = (uint32_t)cubeData.size(),
	});

	mesh.MeshBuffers.IndexBuffer = GL_NONE;
	glCreateBuffers(1, &mesh.MeshBuffers.VertexBuffer);
	glNamedBufferStorage(mesh.MeshBuffers.VertexBuffer, cubeData.size() * sizeof(SVertex), cubeData.data(), GL_DYNAMIC_STORAGE_BIT);
	auto deferredDelete = Defer([&]()
	{
		glDeleteBuffers(1, &mesh.MeshBuffers.VertexBuffer);
	});

	auto pvpShader = CAssetLoader::LoadShaderProgram("Shaders/pvpShader.vert", "Shaders/pvpShader.frag");
	if (!pvpShader)
		std::abort();

	Engine->RenderFunc = [&](float deltaTime)
	{
		constexpr float MaxDeltaTime = 0.2f;
		deltaTime = glm::min(deltaTime, MaxDeltaTime);
		const float aspectRatio = Engine->Viewport.AspectRatio;
		const float time = Engine->CurrentTime;
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		shader.Use();

		std::for_each(lights.begin(), lights.end(), [&](auto& light)
		{
			int i = &light - lights.data();
			std::string prefix = std::format("pointLights[{}].", i);
			shader.SetUniform(prefix + "position", light.Transform.GetPosition());
			shader.SetUniform(prefix + "ambient", vec3(0.1f));
			shader.SetUniform(prefix + "diffuse", vec3(0.5f));
			shader.SetUniform(prefix + "specular", vec3(1.f));
			shader.SetUniform(prefix + "constant", 1.f);
			shader.SetUniform(prefix + "linear", 0.09f);
			shader.SetUniform(prefix + "quadratic", 0.032f);

		});

		shader.SetUniform("spotLight.position", camera.Transform.GetPosition());
		shader.SetUniform("spotLight.direction", camera.GetFrontVector());
		shader.SetUniform("spotLight.ambient", vec3(0.1f));
		shader.SetUniform("spotLight.diffuse", vec3(0.5f));
		shader.SetUniform("spotLight.specular", vec3(1.f));
		shader.SetUniform("spotLight.constant", 1.f);
		shader.SetUniform("spotLight.linear", 0.09f);
		shader.SetUniform("spotLight.quadratic", 0.032f);
		shader.SetUniform("spotLight.innerCutoff", glm::radians(12.f));
		shader.SetUniform("spotLight.outerCutoff", glm::radians(17.f));

		shader.SetUniform("viewPos", camera.Transform.GetPosition());
		shader.SetUniform("worldToCamera", camera.UpdateAndGetViewMatrix());
		shader.SetUniform("cameraToPerspective", camera.GetProjectionMatrix());
		shader.SetUniform("material.specular", vec3(0.5f, 0.5f, 0.5f));
		shader.SetUniform("material.shininess", 32.f);

		glBindVertexArray(cubeVao);
		glBindTextureUnit(0, crateDiffuse);
		glBindTextureUnit(1, crateSpecular);
		shader.SetUniform("material.diffuseMap", 0);
		shader.SetUniform("material.diffuseMap", 1);

		for (int i = 0; i < 8; i++)
		{
			vec3 color(0.0f);
			auto angles = cubes[i].Transform.GetAngles();
			vec3 pos(0.0f);
			if (!angles)
				continue;
			for (int j = 0; j < 3; j++)
			{
				color[j] = float((i >> j) & 1);
				if ((i >> j) & 1)
				{
					angles.value()[j] += deltaTime;
					pos[j] = 3.f * (i / 8 + 1);
				}
			}
			cubes[i].Transform.SetRotation(*angles);
			cubes[i].Transform.SetPosition(pos);
			// shader.SetUniform("material.ambient", color);
			// shader.SetUniform("material.diffuse", color);
			shader.SetUniform("localToWorld", cubes[i].Transform.GetMatrix());
			if (i > 0)
				glDrawArrays(GL_TRIANGLES, 0, sizeof(vertices) / (8 * sizeof(float)));
		}

		lightShader.Use();
		lightShader.SetUniform("worldToCamera", camera.GetViewMatrix());
		lightShader.SetUniform("cameraToPerspective", camera.GetProjectionMatrix());
		glBindVertexArray(lightVao);
		for (int i = 0; i < (int)lights.size(); i++)
		{
			lightShader.SetUniform("localToWorld", lights[i].Transform.GetMatrix());
			glDrawArrays(GL_TRIANGLES, 0, sizeof(vertices) / (8 * sizeof(float)));
		}

		pvpShader->Use();
		pvpShader->SetUniform("localToWorld", cubes[0].Transform.GetMatrix());
		pvpShader->SetUniform("worldToCamera", camera.UpdateAndGetViewMatrix());
		pvpShader->SetUniform("cameraToPerspective", camera.GetProjectionMatrix());

		// glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		// glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glBindVertexArray(0);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, mesh.MeshBuffers.VertexBuffer);
		for (SGeoSurface const& surface : mesh.Surfaces)
		{
			if (mesh.MeshBuffers.IndexBuffer != GL_NONE)
			{
				glDrawElements(GL_TRIANGLES, surface.Count, GL_UNSIGNED_INT, (void*)surface.StartIndex);
			}
			else
			{
				glDrawArrays(GL_TRIANGLES, surface.StartIndex, surface.Count);
			}
		}

	};
	Engine->MainLoop();

	glDeleteVertexArrays(2, vaos);
	glDeleteBuffers(1, &cubeBuffer);

	glfwTerminate();
	return 0;
}
#endif
