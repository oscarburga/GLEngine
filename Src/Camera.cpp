#include "Camera.h"

#include <iostream>
#include "EngineMath.h"
#include <GLFW/glfw3.h>
#include "WorldContainers.h"

Camera::Camera(const vec3& _pos, const vec3& _lookAtLoc) : pos(_pos)
{
    // If this assertion fails, camera is trying to look straight up/down. We don't support this.
    assert(abs(glm::dot(glm::normalize(_lookAtLoc - _pos), World::up)) < (1 - 1e-6));
    front = glm::normalize(_lookAtLoc - _pos);
    const vec3 right = glm::cross(front, World::up); // Both unit vectors, don't need renormalize
    up = glm::cross(right, front); // Both unit vectors, ddon't need renormalize
    lookAt = glm::lookAt(_pos, _lookAtLoc, up);
    const float pitchr = asin(front.y);
    pitch = glm::degrees(pitchr);
    // sin(yaw) * cos(pitch) = front.z = 0.5 * (sin(yaw+pitch) + sin(yaw-pitch))
    // cos(yaw) * cos(pitch) = front.x = 0.5 * (cos(yaw-pitch) + cos(yaw+pitch))

    // front.z = sin(yaw) * cos(pitch)
    // sin(yaw) = front.z / cos(pitch)
    // yaw = asin(front.z / cos(pitch))
    const float yawr = asin(front.z / cos(pitchr));
    yaw = glm::degrees(yawr);
    // TODO: validate the resulting values of yaw and pitch produce a vector that points in the direction of front
    // have to multiply yaw by -1 if it doesnt.
}

mat4& Camera::UpdateAndGetWorldToCamera()
{
    lookAt = glm::lookAt(pos, pos + front, up);
    return lookAt;
}

void Camera::UpdateAxesFromYawPitch()
{
    const float yawr = glm::radians(yaw);
    const float pitchr = glm::radians(pitch);
    front.x = cos(yawr) * cos(pitchr);
    front.y = sin(pitchr);
    front.z = sin(yawr) * cos(pitchr);
    front = glm::normalize(front);
    const vec3 right = glm::normalize(glm::cross(front, World::up));
    up = glm::cross(right, front);
}

void Camera::HandleInput(GLFWwindow* window, float deltax, float deltay, float deltaTime)
{
	vec3 moveDir(0.0f);
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		moveDir += front;
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		moveDir -= front;
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		moveDir -= GetRightVector();
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		moveDir += GetRightVector(); 

	pos += abs(glm::dot(moveDir, moveDir)) > 1e-8f ?
		(speed * deltaTime * glm::normalize(moveDir)) :
		vec3(0.0f);

	moveDir = vec3(0.0f);
	if (glfwGetKey(window, GLFW_KEY_E))
		moveDir += up;
	if (glfwGetKey(window, GLFW_KEY_Q))
		moveDir -= up;

	pos += abs(glm::dot(moveDir, moveDir)) > 1e-8f ?
		(speed * deltaTime * glm::normalize(moveDir)) :
		vec3(0.0f);

	yaw += deltax * deltaTime * yawSens;
	pitch -= deltay * deltaTime * pitchSens;
	pitch = glm::clamp(pitch, -89.f, 89.f);

	UpdateAxesFromYawPitch();
	std::cout << yaw << " " << pitch << std::endl;
}
