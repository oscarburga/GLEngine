#include "Engine.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <cassert>
#include <iostream>

CEngine* CEngine::Create()
{
	Engine = new CEngine();
	return Engine;
}

void CEngine::MainLoop()
{
	const float newTime = (float)glfwGetTime();
	const float deltaTime = newTime - CurrentTime;
	while (!glfwWindowShouldClose(Viewport.Window))
	{
		glfwPollEvents();
		ProcessInput();
		glfwSwapBuffers(Viewport.Window);
	}
}

void CEngine::ProcessInput()
{
	if (glfwGetKey(Viewport.Window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(Viewport.Window, true);

}

CEngine::CEngine()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	Viewport.Window = glfwCreateWindow(Viewport.SizeX, Viewport.SizeY, "GLEngine", nullptr, nullptr);
	if (Viewport.Window == nullptr)
	{
		std::cerr << "Failed to create glfw window\n";
		glfwTerminate();
		std::abort();
	}
	glfwMakeContextCurrent(Viewport.Window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cerr << "Failed to init GLAD\n";
		std::abort();
	}

	glfwSetFramebufferSizeCallback(Viewport.Window, &CEngine::OnWindowResize);
	glfwSetInputMode(Viewport.Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	{
		int numAttributes;
		glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &numAttributes);
		std::cout << "Max number of vertex attributes: " << numAttributes << " 4-component vertex attributes\n";
	}
}

void CEngine::OnWindowResize(GLFWwindow* window, int width, int height)
{
	CEngine* Engine = CEngine::Get();
	Engine->Viewport = {
		.Window = window,
		.SizeX = (uint32_t)width,
		.SizeY = (uint32_t)height,
		.AspectRatio = float(width) / float(height)
	};
	glViewport(0, 0, width, height);
}
