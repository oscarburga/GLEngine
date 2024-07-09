#pragma once

struct GLFWwindow;
struct SSceneData;

class SGlCamera 
{
public:
	float FOV = glm::radians(80.f); // Field of view in radians
	float Speed = 5.0f;
	float YawSens = 1.f;
	float PitchSens = 1.f;
	float NearPlane = 0.1f;
	float FarPlane = 100.f;
	glm::vec3 Position {};
	glm::quat Rotation {};

	void CalcViewMatrix(glm::mat4& outMat) const;
	void CalcProjMatrix(glm::mat4& outMat) const;
	void UpdateSceneData(SSceneData& sceneData);
	// glm::mat4 GetViewMatrix() const { glm::mat4 m; CalcViewMatrix(m); return m; } // View transforms from world space to view/camera space.
	// glm::mat4 GetProjectionMatrix() const { glm::mat4 m; CalcProjMatrix(m); return m; } // Projection transforms from camera/view space to clip space (applies perspective)

	// TEMP: input, eventually this should be moved somewhere else. SGlCamera should just hold the data for the camera used for rendering.
	float Yaw = 0.0f;
	float Pitch = 0.0f;
	void UpdateCameraFromInput(GLFWwindow* window, float deltax, float deltay, float deltaTime);
};
