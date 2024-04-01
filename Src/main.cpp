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
	const float vertices[] = {
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
		 0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
		-0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

		-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f
	};

	const vec3 cubePositions[] = {
		vec3(0.0f,  0.0f,  0.0f),
		vec3(2.0f,  5.0f, -15.0f),
		vec3(-1.5f, -2.2f, -2.5f),
		vec3(-3.8f, -2.0f, -12.3f),
		vec3(2.4f, -0.4f, -3.5f),
		vec3(-1.7f,  3.0f, -7.5f),
		vec3(1.3f, -2.0f, -2.5f),
		vec3(1.5f,  2.0f, -2.5f),
		vec3(1.5f,  0.2f, -1.5f),
		vec3(-1.3f,  1.0f, -1.5f)
	};

	// Vertex array objects store all the calls to glEnableVertexAttribArray or glDisableVertexAttribArray, glVertexAttribPointer, and also stores the buffer objects associated.
	// Use them to not have to re-specify these attributes every time you want to use a certain buffer with a layout. 
	GLuint vaos[2];
	glCreateVertexArrays(2, vaos);
	auto [VAO, lightVao] = vaos;
	GLuint buffers[3];
	glCreateBuffers(3, buffers);
	// glGenBuffers(2, buffers); // Create a buffer and get a handle to it
	// auto [VBO, texCoordBuffer, IBO] = buffers; 
	GLuint VBO = buffers[0];

	glNamedBufferData(VBO, sizeof(vertices), vertices, GL_STATIC_DRAW);
	// glNamedBufferData(IBO, sizeof(indices), indices, GL_STATIC_DRAW);
	// glVertexArrayElementBuffer(VAO, IBO); // Set the VAO to use IndexBufferObject for the element indices.

	// What buffers it uses in each slot for vertex data
	const int vboBindIndex = 0;// , texCoordsBindIndex = 1;
	glVertexArrayVertexBuffer(lightVao, vboBindIndex, VBO, 0, 5 * sizeof(float));
	glEnableVertexArrayAttrib(lightVao, 0);
	glVertexArrayAttribFormat(lightVao, 0, 3, GL_FLOAT, GL_FALSE, 0);
	glVertexArrayAttribBinding(lightVao, 0, vboBindIndex);

	glVertexArrayVertexBuffer(VAO, vboBindIndex, VBO, 0, 5 * sizeof(float));

	glEnableVertexArrayAttrib(VAO, 0); // enable VAO's array attrib 0
	glVertexArrayAttribFormat(VAO, 0, 3, GL_FLOAT, GL_FALSE, 0); // VAO's array attrib 0 has 3 floats,
	glVertexArrayAttribBinding(VAO, 0, vboBindIndex); // VAO's array attrib 0 reads from the buffer bound at vboBindIndex (0)

	glEnableVertexArrayAttrib(VAO, 1);
	glVertexArrayAttribFormat(VAO, 1, 2, GL_FLOAT, GL_FALSE, 3 * sizeof(float)); // array attrib has 3 floats, starting after 3 floats (from attrib 0)
	glVertexArrayAttribBinding(VAO, 1, vboBindIndex); // VAO's array attrib 1 reads from the buffer bound at vboBindIndex (0)

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
			std::cerr << "failed to load texture\n";
			std::abort();
		}
	}
	auto [woodTexture, faceTexture] = textures;
	
	Shader shader("Shaders/shader.vert", "Shaders/shader.frag");
	shader.use();
	// glBindVertexArray(VAO);
	// glBindTextureUnit(0, woodTexture);
	// glBindTextureUnit(1, faceTexture);
	// shader.SetUniform("woodTexture", int(0));
	// shader.SetUniform("faceTexture", int(1));

	Shader lightShader("Shaders/lightningShader.vert", "Shaders/lightningShader.frag");
	lightShader.use();
	// glPolygonMode(GL_BACK, GL_LINE);
	glEnable(GL_DEPTH_TEST);
	float lastFrameTime = (float)glfwGetTime();
	Camera& camera = WorldContainers::InputProcessors.emplace_back(Camera());
	Engine->RenderFunc = [&](float deltaTime)
	{
		const float aspectRatio = Engine->Viewport.AspectRatio;
		const float time = Engine->CurrentTime;
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		shader.use();
		shader.SetUniform("objectColor", vec3(1.0f, 0.5f, 0.31f));
		shader.SetUniform("lightColor", vec3(1.0f, 1.0f, 1.0f));
		mat4 cameraToPerspective = glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 100.f);
		shader.SetUniform("worldToCamera", camera.UpdateAndGetWorldToCamera());
		shader.SetVarUniform(cameraToPerspective);
		mat4 localToWorld = mat4(1.0f);
		shader.SetVarUniform(localToWorld);

		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, sizeof(vertices) / (5 * sizeof(float)));

		lightShader.use();
		localToWorld = glm::translate(localToWorld, vec3(1.2f, 1.0f, 2.0f));
		// localToWorld = glm::rotate(localToWorld, ...);
		localToWorld = glm::scale(localToWorld, vec3(0.2f));
		lightShader.SetUniform("worldToCamera", camera.GetWorldToCamera());
		lightShader.SetVarUniform(cameraToPerspective);
		lightShader.SetVarUniform(localToWorld);

		glBindVertexArray(lightVao);
		glDrawArrays(GL_TRIANGLES, 0, sizeof(vertices) / (5 * sizeof(float)));
	};
	Engine->MainLoop();

	glDeleteVertexArrays(2, vaos);
	glDeleteBuffers(3, buffers);

	glfwTerminate();
	return 0;
}
