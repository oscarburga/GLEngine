#include "ImguiTools.h"
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

IImguiObject::IImguiObject()
{
	CImguiManager::Get()->ImguiObjects.insert(this);
}

IImguiObject::~IImguiObject()
{
	CImguiManager::Get()->ImguiObjects.erase(this);
}

void CImguiManager::Init(GLFWwindow* window)
{
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
	// io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad; // Enable Gamepad Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; // IF using Docking Branch

	// Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForOpenGL(window, true); // Second param install_callback=true will install GLFW callbacks and chain to existing ones.
	ImGui_ImplOpenGL3_Init();
	bIsInitialized = true;
}

void CImguiManager::Cleanup()
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	ImguiObjects.clear();
	bIsInitialized = false;
}

bool CImguiManager::IsCapturingInput()
{
	ImGuiIO& io = ImGui::GetIO();
	return io.WantCaptureKeyboard || io.WantCaptureMouse;
}

void CImguiManager::SetCapturingInput(bool bCaptureInput)
{
	constexpr ImGuiConfigFlags DisableKbmFlags = ImGuiConfigFlags_NoMouse | ImGuiConfigFlags_NoKeyboard;
	ImGuiIO& io = ImGui::GetIO();
	if (bCaptureInput)
		io.ConfigFlags &= ~DisableKbmFlags;
	else
		io.ConfigFlags |= DisableKbmFlags;
}

void CImguiManager::NewFrame(float deltaTime)
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	// ImGui::ShowDemoWindow(); // Show demo window! :)
}

void CImguiManager::EndNewFrame(float deltaTime)
{
	for (IImguiObject* obj : ImguiObjects)
	{
		if (obj->ShouldShowImguiPanel())
			obj->ShowImguiPanel();
	}
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
