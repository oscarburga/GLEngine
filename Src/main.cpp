#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include <iostream>
#include <format>

#include "Assets/AssetLoader.h"
#include "Engine.h"
#include "Math/EngineMath.h"
#include <Render/GlRenderer.h>
#include "Render/SceneGraph.h"
#include "Render/Skinning.h"
#include "Utils/RefIgnore.h"

int main(int argc, char** argv)
{
	CEngine* engine = CEngine::Create();

	auto gltf = CAssetLoader::Get()->LoadGLTFScene("GLTF/ufo_scene/scene.gltf");
	gltf->UserTransform = STransform {
		glm::vec3 {0.0f},
		glm::IQuat, //glm::angleAxis(pi_half, World::Front),
		glm::vec3 { 0.01f }
	};

	auto daji = CAssetLoader::Get()->LoadGLTFScene("GLTF/daji_hok/scene.gltf");
	daji->UserTransform = STransform {
		World::Right * 0.f + World::Front * 5.f,
		glm::IQuat,
		glm::vec3(1.f)
	};
	// Goku's gltf file is kinda kapoot? seems that the skinning matrices add a big scale factor
	// that's not present in their actual node transforms. Frustum culling works wanky on him because of it,
	// the nodes actual transforms are wayyy smaller than the rendered skinned mesh
	// To fix this, I'd need to calculate the bounds after applying the skinning matrix to each vertex... 
	// but I won't do that just to get a weirdly-skinned goku model culling properly haha
	auto goku = CAssetLoader::Get()->LoadGLTFScene("GLTF/goku_rigged__animated/scene.gltf");
	goku->UserTransform = STransform {
		World::Right * -2.f + World::Front * 5.f,
		glm::IQuat,
		glm::vec3(1.f)
	};
	auto astronaut = CAssetLoader::Get()->LoadGLTFScene("GLTF/astronaut_rigged_and_animated/scene.glb");
	astronaut->UserTransform = STransform {
		World::Right * 2.f + World::Front * 5.f,
		glm::IQuat,
		glm::vec3(0.25f)
	};
	auto dajiSkin = daji->Skins.begin()->second;
	auto gokuSkin = goku->Skins.begin()->second;
	auto astroSkin = astronaut->Skins.begin()->second;
	assert(gltf && daji && dajiSkin && astronaut && astroSkin);
	dajiSkin->Animator->PlayAnimation("Idleshow", true);
	gokuSkin->Animator->PlayAnimation("Idle", true);
	astroSkin->Animator->PlayAnimation("astro_bones|idle_1", true);
	CGlRenderer* renderer = CGlRenderer::Get();
	engine->PreRenderFuncs.emplace_back([&, totalTime = 0.f](float deltaTime) mutable
	{
		gltf->Draw(STransform {}, *renderer->MainDrawContext);
		dajiSkin->Animator->UpdateAnimation(deltaTime);
		gokuSkin->Animator->UpdateAnimation(deltaTime);
		astroSkin->Animator->UpdateAnimation(deltaTime);
		daji->Draw(STransform{}, *renderer->MainDrawContext);
		goku->Draw(STransform{}, *renderer->MainDrawContext);
		astronaut->Draw(STransform{}, *renderer->MainDrawContext);
		// CAssetLoader::Get()->AxisMesh->Draw(STransform {}, *renderer->MainDrawContext);
		totalTime += deltaTime;
	});
	engine->MainLoop();

	return 0;
}
