#pragma once

#include <stdint.h>
#include "Utils/FuncVector.h"

struct GLFWwindow;

struct SViewport
{
	GLFWwindow* Window = nullptr;
	uint32_t SizeX = 1280;
	uint32_t SizeY = 720;
	float AspectRatio = 1280.f / 720.f;
};

class CEngine
{
protected:
	void Init();
	CEngine() {};
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

	// Not sure if i'm keeping this or how i'll handle this. 
	// Subclassing + virtual function makes the most sense but we'll see.
	TFuncVector<float> PreRenderFuncs;

protected:
	static void OnWindowResize(GLFWwindow* window, int width, int height);
	static CEngine* Engine;
	bool bGlfwCapturesMouse = false;
};
