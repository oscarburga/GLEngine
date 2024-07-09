#pragma once

struct GLFWwindow;

class SGlCamera 
{
	float FOV = glm::radians(80.f); // Field of view in radians
	mutable glm::mat4 ViewMatrix;
	glm::mat4 ProjMatrix;
public:
	SGlCamera() { SetFOV(FOV); };
	float Speed = 5.0f;
	float YawSens = 1.f;
	float PitchSens = 1.f;
	float NearPlane = 0.1f;
	float FarPlane = 100.f;
	const glm::mat4& GetViewMatrix() const { return ViewMatrix; } // View transforms from world space to view/camera space.
	const glm::mat4& UpdateAndGetViewMatrix() const;
	const glm::mat4& GetProjectionMatrix() { return ProjMatrix; } // Projection transforms from camera/view space to clip space (applies perspective)
	void SetFOV(float _fov);

	// TEMP: input, eventually this should be moved somewhere else. SGlCamera should just hold the data for the camera used for rendering.
	float Yaw = 0.0f;
	float Pitch = 0.0f;
	glm::vec3 Position {};
	glm::quat Rotation {};
	void UpdateCameraFromInput(GLFWwindow* window, float deltax, float deltay, float deltaTime);
};
