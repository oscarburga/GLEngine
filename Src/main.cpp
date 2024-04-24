#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include "Shader.h"
#include "EngineMath.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "RefIgnore.h"
#include "Engine.h"
#include "Camera.h"
#include "WorldContainers.h"


int main(int argc, char** argv)
{
	CEngine* Engine = CEngine::Create();

	float vertices[] = {
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		 0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 
		 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 
		 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 
		-0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 

		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,

		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,

		 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
		 0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
		 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
		 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
		 0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,

		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f
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
	glVertexArrayVertexBuffer(lightVao, vboBindIndex, cubeBuffer, 0, 6 * sizeof(float));
	glEnableVertexArrayAttrib(lightVao, 0);
	glVertexArrayAttribFormat(lightVao, 0, 3, GL_FLOAT, GL_FALSE, 0);
	glVertexArrayAttribBinding(lightVao, 0, vboBindIndex);

	glVertexArrayVertexBuffer(cubeVao, vboBindIndex, cubeBuffer, 0, 6 * sizeof(float));

	glEnableVertexArrayAttrib(cubeVao, 0); // enable VAO's array attrib 0
	glVertexArrayAttribFormat(cubeVao, 0, 3, GL_FLOAT, GL_FALSE, 0); // VAO's array attrib 0 has 3 floats,
	glVertexArrayAttribBinding(cubeVao, 0, vboBindIndex); // VAO's array attrib 0 reads from the buffer bound at vboBindIndex (0)

	glEnableVertexArrayAttrib(cubeVao, 1);
	glVertexArrayAttribFormat(cubeVao, 1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float));
	glVertexArrayAttribBinding(cubeVao, 1, vboBindIndex);

	// Textures
	GLuint textures[2];
	const char* paths[2] = { "Textures/container.jpg", "Textures/awesomeface.png" };
	GLenum inputFormat[2] = { GL_RGB, GL_RGBA };
	glCreateTextures(GL_TEXTURE_2D, 2, textures);
	stbi_set_flip_vertically_on_load(true);
	for (int i = 0; i < 2; i++)
	{
		int w, h, c;
		if (stbi_uc* texData = stbi_load(paths[i], &w, &h, &c, 0))
		{
			GLuint texture = textures[i];
			const int numLevels = 1 + (int)floor(log2(std::max(w, h))); // TODO use count leading zero to calculate num mipmaps
			glTextureParameteri(texture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
			glTextureParameteri(texture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
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
			std::cerr << "failed to load texture " << paths[i] << "\n";
			std::abort();
		}
	}
	auto [woodTexture, faceTexture] = textures;
	
	CShader shader("Shaders/shader.vert", "Shaders/shader.frag");
	shader.use();
	// glBindVertexArray(VAO);
	// glBindTextureUnit(0, woodTexture);
	// glBindTextureUnit(1, faceTexture);
	// shader.SetUniform("woodTexture", int(0));
	// shader.SetUniform("faceTexture", int(1));

	CShader lightShader("Shaders/lightningShader.vert", "Shaders/lightningShader.frag");
	lightShader.use();
	// glPolygonMode(GL_BACK, GL_LINE);
	glEnable(GL_DEPTH_TEST);
	float lastFrameTime = (float)glfwGetTime();
	GCamera& camera = WorldContainers::InputProcessors.emplace_back<GCamera>();
	std::vector<GWorldObject> cubes(10);
	for (size_t i = 0; i<cubes.size(); i++)
	{
		cubes[i].Transform.SetPosition(cubePositions[i]);
	}
	GWorldObject cube, light;
	light.Transform.SetScale(vec3(0.2f));
	Engine->RenderFunc = [&](float deltaTime)
	{
		constexpr float MaxDeltaTime = 0.2f;
		deltaTime = glm::min(deltaTime, MaxDeltaTime);
		const float aspectRatio = Engine->Viewport.AspectRatio;
		const float time = Engine->CurrentTime;
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		light.Transform.SetPosition(vec3(1.5f, 1.0f + glm::sin(time), -2.0f));
		shader.use();
		// shader.SetUniform("objectColor", vec3(1.0f, 0.5f, 0.31f));
		shader.SetUniform("light.position", light.Transform.GetPosition());
		shader.SetUniform("light.ambient", vec3(0.1f));
		shader.SetUniform("light.diffuse", vec3(0.5f));
		shader.SetUniform("light.specular", vec3(1.f));
		shader.SetUniform("viewPos", camera.Transform.GetPosition());
		shader.SetUniform("worldToCamera", camera.UpdateAndGetViewMatrix());
		shader.SetUniform("cameraToPerspective", camera.GetProjectionMatrix());
		shader.SetUniform("localToWorld", cube.Transform.GetMatrix());
		shader.SetUniform("material.specular", vec3(0.5f, 0.5f, 0.5f));
		shader.SetUniform("material.shininess", 32.f);
		shader.SetUniform("material.shininess", 32.f);

		glBindVertexArray(cubeVao);
		for (int i = 0; i < 10; i++)
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
			shader.SetUniform("material.ambient", color);
			shader.SetUniform("material.diffuse", color);
			shader.SetUniform("localToWorld", cubes[i].Transform.GetMatrix());
			glDrawArrays(GL_TRIANGLES, 0, sizeof(vertices) / (5 * sizeof(float)));
		}

		lightShader.use();
		lightShader.SetUniform("worldToCamera", camera.GetViewMatrix());
		lightShader.SetUniform("cameraToPerspective", camera.GetProjectionMatrix());
		lightShader.SetUniform("localToWorld", light.Transform.GetMatrix());

		glBindVertexArray(lightVao);
		glDrawArrays(GL_TRIANGLES, 0, sizeof(vertices) / (5 * sizeof(float)));
	};
	Engine->MainLoop();

	glDeleteVertexArrays(2, vaos);
	glDeleteBuffers(1, &cubeBuffer);

	glfwTerminate();
	return 0;
}
