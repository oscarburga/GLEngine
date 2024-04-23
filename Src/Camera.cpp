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
	vec3 angles = Transform.GetAngles().value_or(vec3(0.0f));
	angles.x += deltax * deltaTime * YawSens;
	angles.y += deltay * deltaTime * PitchSens;
	angles.z = 0.0f;
	Transform.SetRotation(angles);
	std::cout << glm::degrees(angles.x) << " " << glm::degrees(angles.y) << " " << glm::degrees(angles.z) << std::endl;
}
