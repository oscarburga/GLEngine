#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <format>

#include "Utils/RefIgnore.h"
#include "Engine.h"
#include "Assets/AssetLoader.h"
#include <Render/GlRenderer.h>
#include "Math/EngineMath.h"

int main(int argc, char** argv)
{
	CEngine* engine = CEngine::Create();
	// auto gltf = CAssetLoader::Get()->LoadGLTFScene("GLTF/anxiety/source/vefq.glb");
	auto gltf = CAssetLoader::Get()->LoadGLTFScene("GLTF/tacos.glb");
	auto anxiety = CAssetLoader::Get()->LoadGLTFScene("GLTF/anxiety/source/vefq.glb");
	assert(gltf && anxiety);
	CGlRenderer* renderer = CGlRenderer::Get();
	engine->PreRenderFuncs.emplace_back([&](float deltaTime)
	{
		//glm::mat4 rot = glm::rotate(glm::mat4(1.f), engine->CurrentTime * 0.5f, World::Right);
		glm::mat4 rot = glm::mat4(1.f);
		gltf->Draw(rot, renderer->MainDrawContext);

		anxiety->Draw(glm::rotate(rot, glm::radians(90.f), World::Up), renderer->MainDrawContext);
		CAssetLoader::Get()->AxisMesh->Draw(glm::mat4(1.f), renderer->MainDrawContext);
	});
	engine->MainLoop();

	return 0;
}
