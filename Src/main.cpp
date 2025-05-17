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
	gltf->UserTransform = STransform {
		glm::vec3 {0.0f},
		glm::IQuat, //glm::angleAxis(pi_half, World::Front),
		glm::vec3 { 0.01f }
	};
	auto anxiety = CAssetLoader::Get()->LoadGLTFScene("GLTF/anxiety/source/vefq.glb");
	anxiety->UserTransform = STransform {
		World::Up * 0.025f + World::Front * 3.f - World::Right * 3.f,
		glm::IQuat,
		glm::vec3(1.f)
	};

	auto catgirl = CAssetLoader::Get()->LoadGLTFScene("GLTF/catgirl_animated.glb");
	catgirl->UserTransform = STransform {
		World::Up * 0.3f + World::Front * 7.f - World::Right * 3.f,
		glm::IQuat,
		glm::vec3(1.f)
	};
	auto catgirlSkin = catgirl->Skins[""];
	assert(gltf && anxiety && catgirl && catgirlSkin);
	catgirlSkin->Animator->PlayAnimation("Idleshow", true);
	CGlRenderer* renderer = CGlRenderer::Get();
	engine->PreRenderFuncs.emplace_back([&, totalTime = 0.f](float deltaTime) mutable
	{
		gltf->Draw(STransform {}, renderer->MainDrawContext);

		// anxiety->Draw(STransform {}, renderer->MainDrawContext);
		catgirlSkin->Animator->UpdateAnimation(deltaTime);
		totalTime += deltaTime;
		// glm::quat q = glm::fromYawPitchRoll(vec3{ totalTime , 0.0f, 0.0f });
		{
			SGlCamera& activeCam = renderer->ActiveCamera;
			glm::vec3 catPos = catgirl->UserTransform.GetPosition();
			glm::vec3 camPos = activeCam.Position;
			camPos.y = catPos.y;
			catgirl->UserTransform.SetRotation(glm::lookAtRotation(catPos, camPos, World::Up));
		}
		// catgirl->UserTransform.SetRotation(q);
		catgirl->Draw(STransform{}, renderer->MainDrawContext);
		CAssetLoader::Get()->AxisMesh->Draw(STransform {}, renderer->MainDrawContext);
	});
	engine->MainLoop();

	return 0;
}
