#include "GlCamera.h"

#include <GLFW/glfw3.h>
#include "GlRenderStructs.h"
#include <iostream>
#include "Engine.h"
#include "Math/EngineMath.h"

void SGlCamera::CalcViewMatrix(glm::mat4& outMat) const
{
	const glm::vec3 front = glm::rotateByQuat(World::Front, Rotation);
	const glm::vec3 up = glm::rotateByQuat(World::Up, Rotation);
    outMat = glm::lookAtLH(Position, Position + front, up);
}

void SGlCamera::CalcProjMatrix(glm::mat4& outMat) const
{
	const float aspectRatio = CEngine::Get()->Viewport.AspectRatio;
	outMat = glm::perspectiveLH(FOV, aspectRatio, NearPlane, FarPlane);
}

void SGlCamera::UpdateSceneData(SSceneData& sceneData)
{
	sceneData.CameraPos = glm::vec4(Position, 1.0f);
	CalcViewMatrix(sceneData.View);
	CalcProjMatrix(sceneData.Proj);
	sceneData.ViewProj = sceneData.Proj * sceneData.View; // * Model * V
}

void SGlCamera::UpdateCameraFromInput(GLFWwindow* window, float deltax, float deltay, float deltaTime)
{
	const glm::vec3 front = glm::rotateByQuat(World::Front, Rotation);
	const glm::vec3 up = glm::rotateByQuat(World::Up, Rotation);
	const glm::vec3 right = glm::rotateByQuat(World::Right, Rotation);
	vec3 moveDir(0.0f);
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		moveDir += front;
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		moveDir -= front;
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		moveDir -= right;
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		moveDir += right; 

	vec3 positionDelta(abs(glm::dot(moveDir, moveDir)) > 1e-8f ?
		(Speed * deltaTime * glm::normalize(moveDir)) :
		vec3(0.0f));

	moveDir = vec3(0.0f);
	if (glfwGetKey(window, GLFW_KEY_E))
		moveDir += up;
	if (glfwGetKey(window, GLFW_KEY_Q))
		moveDir -= up;

	positionDelta += abs(glm::dot(moveDir, moveDir)) > 1e-8f ?
		(Speed * deltaTime * glm::normalize(moveDir)) :
		vec3(0.0f);

	Position += positionDelta;
	Yaw += deltax * deltaTime * YawSens;
	Pitch += deltay * deltaTime * PitchSens;
	constexpr float minPitch = glm::radians(-89.f);
	constexpr float maxPitch = glm::radians(89.f);
	Pitch = glm::clamp(Pitch, minPitch, maxPitch);
	Rotation = glm::fromYawPitchRoll(glm::vec3(Yaw, Pitch, 0.0f));
	// std::cout << glm::degrees(angles.x) << " " << glm::degrees(angles.y) << " " << glm::degrees(angles.z) << std::endl;
}
