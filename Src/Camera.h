#pragma once

#include "EngineMath.h"
#include "World.h"
#include "InputConcepts.h"

struct GLFWwindow;

class Camera
{
	mat4 lookAt;
public:
	vec3 pos;
	vec3 front;
	vec3 up;
	float yaw { 0.0f };
	float pitch { 0.0f };
	float speed { 5.0f };
	float yawSens { 100.01f };
	float pitchSens { 100.01f };

	Camera(const vec3& _pos, const vec3& _lookAtLoc);

	Camera() : Camera(vec3(0.0f, 0.0f, 3.0f), vec3(0.0f, 0.0f, 0.0f)) {}

	vec3 GetRightVector() const { return glm::normalize(glm::cross(front, up)); }
	mat4& GetWorldToCamera() { return lookAt; }
	mat4& UpdateAndGetWorldToCamera();

	void UpdateAxesFromYawPitch();

	void HandleInput(GLFWwindow* window, float deltax, float deltay, float deltaTime);
};

static_assert(HandlesInput<Camera>);
