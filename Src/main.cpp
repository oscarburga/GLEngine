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
	auto gltf = CAssetLoader::Get()->LoadGLTFScene("GLTF/ufo_scene/scene.gltf");
	gltf->UserTransform = STransform::FromScale(glm::vec3 { 0.01f });
	auto anxiety = CAssetLoader::Get()->LoadGLTFScene("GLTF/anxiety/source/vefq.glb");
	anxiety->UserTransform = STransform { 
		glm::translate(glm::scale(glm::IMat, vec3(3.f)),
		World::Up * 0.025f + World::Front - World::Right)
	};
	assert(gltf && anxiety);
	CGlRenderer* renderer = CGlRenderer::Get();
	engine->PreRenderFuncs.emplace_back([&](float deltaTime)
	{
		gltf->Draw(STransform {}, renderer->MainDrawContext);

		anxiety->Draw(STransform {}, renderer->MainDrawContext);
		CAssetLoader::Get()->AxisMesh->Draw(STransform {}, renderer->MainDrawContext);
	});
	engine->MainLoop();

	return 0;
}
