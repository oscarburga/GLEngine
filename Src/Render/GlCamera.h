#pragma once
#include <array>

struct GLFWwindow;
struct SBounds;
struct SFrustum;
struct SSceneData;

// May want to eventually differentiate orthographic from ortho2d (for like UI/quad stuff)...
// enum class ECameraType : uint8_t
// {
// 	Perspective,
// 	Ortho,
// 	Ortho2D
// };

class SGlCamera 
{
public:
	bool bIsPerspective = true;
	float NearPlane = 0.1f;
	float FarPlane = 25.f;
	float PerspectiveFOV = glm::radians(80.f); // Field of view in radians (y-axis)
	glm::vec2 OrthoMinBounds { -1.f, -1.f };
	glm::vec2 OrthoMaxBounds { 1.f, 1.f };
	glm::vec3 Position {};
	glm::quat Rotation {};

	void CalcViewMatrix(glm::mat4& outMat) const;
	void CalcProjMatrix(glm::mat4& outMat) const;
	void CalcFrustum(SFrustum* outFrustum, std::array<glm::vec3, 8>* outCorners) const;
	void UpdateSceneData(SSceneData& sceneData);
	// glm::mat4 GetViewMatrix() const { glm::mat4 m; CalcViewMatrix(m); return m; } // View transforms from world space to view/camera space.
	// glm::mat4 GetProjectionMatrix() const { glm::mat4 m; CalcProjMatrix(m); return m; } // Projection transforms from camera/view space to clip space (applies perspective)

	// TEMP: input, eventually this should be moved somewhere else. SGlCamera should just hold the data for the camera used for rendering.
	float Yaw = 0.0f;
	float Pitch = 0.0f;
	float YawSens = 1.f;
	float PitchSens = 1.f;
	float Speed = 5.0f;
	void UpdateCameraFromInput(GLFWwindow* window, float deltax, float deltay, float deltaTime);
};
