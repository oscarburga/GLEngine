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
    ViewMatrix = glm::lookAt(Position, Position + Front, Up);
    return ViewMatrix;
}

void GCamera::SetFOV(float _fov)
{
	FOV = _fov;
	const float aspectRatio = CEngine::Get()->Viewport.AspectRatio;
	ProjMatrix = glm::perspective(FOV, aspectRatio, NearPlane, FarPlane);
}

void GCamera::HandleInput(GLFWwindow* window, float deltax, float deltay, float deltaTime)
{
	vec3 moveDir(0.0f);
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		moveDir += Front;
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		moveDir -= Front;
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		moveDir -= Right;
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		moveDir += Right; 

	Position += abs(glm::dot(moveDir, moveDir)) > 1e-8f ?
		(Speed * deltaTime * glm::normalize(moveDir)) :
		vec3(0.0f);

	moveDir = vec3(0.0f);
	if (glfwGetKey(window, GLFW_KEY_E))
		moveDir += Up;
	if (glfwGetKey(window, GLFW_KEY_Q))
		moveDir -= Up;

	Position += abs(glm::dot(moveDir, moveDir)) > 1e-8f ?
		(Speed * deltaTime * glm::normalize(moveDir)) :
		vec3(0.0f);

	Angles.x += deltay * deltaTime * PitchSens;
	Angles.y -= deltax * deltaTime * YawSens;

	UpdateRotationFromAngles();
	std::cout << glm::degrees(Angles.y) << " " << glm::degrees(Angles.x) << std::endl;
}
