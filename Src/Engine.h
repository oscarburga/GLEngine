#pragma once

#include <stdint.h>

class GLFWwindow;

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
	float CurrentTime;

	void MainLoop();
	void ProcessInput();

protected:
	CEngine();
	static void OnWindowResize(GLFWwindow* window, int width, int height);
	static CEngine* Engine;
};
