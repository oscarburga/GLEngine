#pragma once

#include <stdint.h>
#include <functional>

struct GLFWwindow;

class CEngine
{
public:
	CEngine(const CEngine&) = delete;
	CEngine(CEngine&&) = delete;
	static CEngine* Create();
	static CEngine* Get() { return Engine; }
	
	struct SViewport
	{
		GLFWwindow* Window = nullptr;
		uint32_t SizeX = 800;
		uint32_t SizeY = 600;
		float AspectRatio = 800.f / 600.f;
	} Viewport;

	float CursorLastX;
	float CursorLastY;
	float CurrentTime;

	void MainLoop();
	void ProcessInput(float deltaTime);

	// TEMP: to keep progressing through learnopengl before making renderable concepts,
	// just set a RenderFunc from main
	std::function<void(float)> RenderFunc;

protected:
	CEngine();
	static void OnWindowResize(GLFWwindow* window, int width, int height);
	static CEngine* Engine;
};
