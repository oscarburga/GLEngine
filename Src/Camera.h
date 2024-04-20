#pragma once

#include "WorldObject.h"
#include "InputConcepts.h"

struct GLFWwindow;

class GCamera : public GWorldObject
{
	mutable mat4 ViewMatrix;
	mat4 ProjMatrix;
	float FOV = glm::radians(80.f); // Field of view in radians
public:
	GCamera();
	float Speed = 5.0f;
	float YawSens = 0.6f;
	float PitchSens = 0.6f;
	float NearPlane = 0.1f;
	float FarPlane = 100.f;
	const mat4& GetViewMatrix() const { return ViewMatrix; } // View transforms from world space to view/camera space.
	const mat4& UpdateAndGetViewMatrix() const;
	const mat4& GetProjectionMatrix() { return ProjMatrix; } // Projection transforms from camera/view space to clip space (applies perspective)
	void SetFOV(float _fov);

	void HandleInput(GLFWwindow* window, float deltax, float deltay, float deltaTime);
};

static_assert(HandlesInput<GCamera>);
