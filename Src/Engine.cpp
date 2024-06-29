#include "Engine.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <cassert>
#include <iostream>
#include <format>
#include "WorldContainers.h"

namespace
{
	/*
	* Shamelessly copied from https ://learnopengl.com/In-Practice/Debugging
	*/ 
	void APIENTRY GlDebugCallback(GLenum source, GLenum type, unsigned int id, GLenum severity, GLsizei length, const char* message, const void* userParam)
	{
		// ignore non-significant error/warning codes
		if(id == 131169 || id == 131185 || id == 131218 || id == 131204) 
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

CEngine* CEngine::Engine = nullptr;

CEngine::CEngine()
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

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cerr << "Failed to init GLAD\n";
		std::abort();
	}

	glfwSetFramebufferSizeCallback(Viewport.Window, &CEngine::OnWindowResize);
	glfwSetInputMode(Viewport.Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	{
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
	}
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
