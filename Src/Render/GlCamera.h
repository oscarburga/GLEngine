#pragma once
#include <array>

struct GLFWwindow;
struct SSceneData;
struct SBounds;

struct SPlane
{
	glm::vec3 Normal { 0.f, 1.f, 0.f };
	glm::vec3 Point { 0.0f, 0.0f, 0.0f };
	float GetSignedDistance(glm::vec3 point);
	bool IsSphereOnPlane(glm::vec3 sphereLoc, float radius); // IsSphereOnHalfspace would be more accurate but w/e...
};

/*
* For left/right/bottom/top, the name corresponds to the actual plane in the frustrum.
* Left plane is the left plane, with a normal pointing towards the right.
* Bottom plane is the bottom plane, with a normal pointing upwards, etc.
*/
struct SFrustum
{
	union
	{
		struct
		{
			SPlane Near;
			SPlane Far;
			SPlane Left;
			SPlane Right;
			SPlane Bottom;
			SPlane Top;
		};
		std::array<SPlane, 6> Planes {};
	};
	bool IsSphereInFrustum(const SBounds& bounds, const glm::mat4& transform);
};

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
	float FarPlane = 100.f;
	float PerspectiveFOV = glm::radians(80.f); // Field of view in radians (y-axis)
	glm::vec2 OrthoSize { 1.f, 1.f };
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
