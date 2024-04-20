#include "Engine.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <cassert>
#include <iostream>
#include "WorldContainers.h"

CEngine* CEngine::Engine = nullptr;

CEngine* CEngine::Create()
{
	assert(!Engine);
	Engine = new CEngine();
	return Engine;
}

void CEngine::Destroy()
{
	delete Engine;
}

void CEngine::MainLoop()
{
	CurrentTime = (float)glfwGetTime();
	assert(RenderFunc);
	while (!glfwWindowShouldClose(Viewport.Window))
	{
		const float newTime = (float)glfwGetTime();
		const float deltaTime = newTime - CurrentTime;
		CurrentTime = newTime;
		glfwPollEvents();
		ProcessInput(deltaTime);
		RenderFunc(deltaTime);
		glfwSwapBuffers(Viewport.Window);
	}
}

void CEngine::ProcessInput(float deltaTime)
{
	if (glfwGetKey(Viewport.Window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(Viewport.Window, true);

	double xpos, ypos;
	glfwGetCursorPos(Viewport.Window, &xpos, &ypos);
	static float lastx = (float)xpos;
	static float lasty = (float)ypos;

	float deltax = (float(xpos) - lastx);
	float deltay = (float(ypos) - lasty);

	lastx = (float)xpos;
	lasty = (float)ypos;

	WorldContainers::InputProcessors.apply([&](HandlesInput auto& inputProcessor)
	{
		inputProcessor.HandleInput(Viewport.Window, deltax, deltay, deltaTime);
	});
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
	CurrentTime = (float)glfwGetTime();
}

CEngine::~CEngine()
{
	glfwTerminate();
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
