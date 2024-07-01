#pragma once

#include <stdint.h>
#include <functional>

struct GLFWwindow;

struct SViewport
{
	GLFWwindow* Window = nullptr;
	uint32_t SizeX = 800;
	uint32_t SizeY = 600;
	float AspectRatio = 800.f / 600.f;
};

class CEngine
{
protected:
	CEngine();
	~CEngine();
public:
	CEngine(const CEngine&) = delete;
	CEngine(CEngine&&) = delete;
	static CEngine* Create();
	static void Destroy();
	inline static CEngine* Get() { return Engine; }

	SViewport Viewport;
	
	float CursorLastX = 0.0f;
	float CursorLastY = 0.0f;
	float CurrentTime = 0.0f;

	void MainLoop();
	void ProcessInput(float deltaTime);

	// TEMP: to keep progressing through learnopengl before making renderable concepts,
	// just set a RenderFunc from main
	std::function<void(float)> RenderFunc;

protected:
	static void OnWindowResize(GLFWwindow* window, int width, int height);
	static CEngine* Engine;
};
