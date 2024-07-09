#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <format>

#include "Utils/RefIgnore.h"
#include "Engine.h"
#include "Assets/AssetLoader.h"
#include <Render/GlRenderer.h>

int main(int argc, char** argv)
{
	CEngine* engine = CEngine::Create();
	auto gltf = CAssetLoader::Get()->LoadGLTFScene("GLTF/shiba/scene.gltf");
	assert(gltf);
	CGlRenderer* renderer = CGlRenderer::Get();
	engine->PreRenderFuncs.emplace_back([&](float deltaTime)
	{
		gltf->Draw(glm::mat4(1.f), renderer->MainDrawContext);
		CAssetLoader::Get()->AxisMesh->Draw(glm::mat4(1.f), renderer->MainDrawContext);
	});
	engine->MainLoop();

	return 0;
}
