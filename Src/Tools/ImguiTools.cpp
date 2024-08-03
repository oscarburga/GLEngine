#include "ImguiTools.h"
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

CImguiManager* CImguiManager::Instance = nullptr;

CImguiManager::CImguiManager()
{

}

CImguiManager::~CImguiManager()
{
	Cleanup();
}

CImguiManager* CImguiManager::Create(GLFWwindow* window)
{
	if (!Instance)
	{
		Instance = new CImguiManager();
		Instance->Init(window);
	}
	return Instance;
}

void CImguiManager::Destroy()
{
	if (Instance)
	{
		Instance->Cleanup();
		delete Instance;
	}
}

void CImguiManager::Init(GLFWwindow* window)
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // IF using Docking Branch

	// Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForOpenGL(window, true);          // Second param install_callback=true will install GLFW callbacks and chain to existing ones.
	ImGui_ImplOpenGL3_Init();
}

void CImguiManager::Cleanup()
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

bool CImguiManager::IsCapturingInput()
{
	ImGuiIO& io = ImGui::GetIO();
	return io.WantCaptureKeyboard || io.WantCaptureMouse;
}

void CImguiManager::NewFrame(float deltaTime)
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	ImGui::ShowDemoWindow(); // Show demo window! :)
}

void CImguiManager::EndNewFrame(float deltaTime)
{
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
