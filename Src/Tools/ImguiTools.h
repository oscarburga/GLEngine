#pragma once

struct GLFWwindow;

class CImguiManager
{
	friend class CEngine;
	CImguiManager();
	~CImguiManager();
	static CImguiManager* Instance;
	static CImguiManager* Create(GLFWwindow* window);
	static void Destroy();
	void Init(GLFWwindow* window);
	void Cleanup();
	bool IsCapturingInput();
	void NewFrame(float deltaTime);
	void EndNewFrame(float deltaTime);
public:
	inline static CImguiManager* Get() { return Instance; };
};

class IImguiConfig
{
	void ShowImguiPanel();
};
