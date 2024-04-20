#include "Camera.h"

#include <iostream>
#include "EngineMath.h"
#include <GLFW/glfw3.h>
#include "WorldContainers.h"
#include "Engine.h"

GCamera::GCamera()
{
	SetFOV(FOV);
}

const mat4& GCamera::UpdateAndGetViewMatrix() const
{
    ViewMatrix = glm::lookAtLH(Transform.GetPosition(), Transform.GetPosition() + GetFrontVector(), GetUpVector());
    return ViewMatrix;
}

void GCamera::SetFOV(float _fov)
{
	FOV = _fov;
	const float aspectRatio = CEngine::Get()->Viewport.AspectRatio;
	ProjMatrix = glm::perspectiveLH(FOV, aspectRatio, NearPlane, FarPlane);
}

void GCamera::HandleInput(GLFWwindow* window, float deltax, float deltay, float deltaTime)
{
	vec3 moveDir(0.0f);
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		moveDir += GetFrontVector();
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		moveDir -= GetFrontVector();
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		moveDir -= GetRightVector();
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		moveDir += GetRightVector(); 

	vec3 positionDelta(abs(glm::dot(moveDir, moveDir)) > 1e-8f ?
		(Speed * deltaTime * glm::normalize(moveDir)) :
		vec3(0.0f));

	moveDir = vec3(0.0f);
	if (glfwGetKey(window, GLFW_KEY_E))
		moveDir += GetUpVector();
	if (glfwGetKey(window, GLFW_KEY_Q))
		moveDir -= GetUpVector();

	positionDelta += abs(glm::dot(moveDir, moveDir)) > 1e-8f ?
		(Speed * deltaTime * glm::normalize(moveDir)) :
		vec3(0.0f);

	Transform.SetPosition(Transform.GetPosition() + positionDelta);


	static float yaw = 0, pitch = 0;
	const float deltaYaw = deltax * deltaTime * YawSens;
	const float deltaPitch = deltay * deltaTime * PitchSens;
	yaw += deltaYaw;
	pitch += deltaPitch;
	quat rot = glm::fromYawPitch(yaw, pitch);
	Transform.SetRotation(rot);
	std::cout << glm::degrees(yaw) << " " << glm::degrees(pitch) << std::endl;
}
