#pragma once
#include <unordered_set>

struct GLFWwindow;

// This interface should never be constructed/destroyed directly, so using non-virtual protected c/dtors
class IImguiObject
{
public:
	bool bShowImguiPanel = false;
	virtual bool ShouldShowImguiPanel() { return bShowImguiPanel; }
	virtual void ShowImguiPanel() = 0;
protected:
	IImguiObject();
	~IImguiObject();
};

class CImguiManager final
{
	friend class CEngine;
	friend class IImguiObject;
	void Init(GLFWwindow* window);
	void Cleanup();
	bool IsCapturingInput();
	void SetCapturingInput(bool bCaptureInput);
	void NewFrame(float deltaTime);
	void EndNewFrame(float deltaTime);

protected:
	bool bIsInitialized = false;
	std::unordered_set<IImguiObject*> ImguiObjects;

public:
	inline static CImguiManager* Get() { 
		static CImguiManager Manager {};
		return &Manager; 
	};
};
