#include "Engine.h"

#include <GLFW/glfw3.h>
#include <Render/GlRenderer.h>

#include <cassert>
#include <iostream>
#include <format>

CEngine* CEngine::Engine = nullptr;

void CEngine::Init()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);

	Viewport.Window = glfwCreateWindow(Viewport.SizeX, Viewport.SizeY, "GLEngine", nullptr, nullptr);
	if (Viewport.Window == nullptr)
	{
		std::cerr << "Failed to create glfw window\n";
		glfwTerminate();
		std::abort();
	}
	glfwMakeContextCurrent(Viewport.Window);
	glfwSetFramebufferSizeCallback(Viewport.Window, &CEngine::OnWindowResize);
	glfwSetInputMode(Viewport.Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	CGlRenderer::Create((GlFunctionLoaderFuncType)glfwGetProcAddress);

	CurrentTime = (float)glfwGetTime();
}

CEngine::~CEngine()
{
	CGlRenderer::Destroy();
	glfwTerminate();
}

CEngine* CEngine::Create()
{
	assert(!Engine);
	Engine = new CEngine();
	Engine->Init();
	return Engine;
}

void CEngine::Destroy()
{
	delete Engine;
}

void CEngine::MainLoop()
{
	CurrentTime = (float)glfwGetTime();
	while (!glfwWindowShouldClose(Viewport.Window))
	{
		const float newTime = (float)glfwGetTime();
		const float deltaTime = newTime - CurrentTime;
		CurrentTime = newTime;
		glfwPollEvents();
		ProcessInput(deltaTime);
		PreRenderFuncs.Execute(deltaTime);
		CGlRenderer::Get()->RenderScene(deltaTime);
		//RenderFunc(deltaTime);
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

	CGlRenderer::Get()->ActiveCamera.UpdateCameraFromInput(Viewport.Window, deltax, deltay, deltaTime);
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
	CGlRenderer::Get()->OnWindowResize(Engine, Engine->Viewport);
}
