#include "GlCamera.h"

#include <GLFW/glfw3.h>
#include "GlRenderStructs.h"
#include <iostream>
#include "Engine.h"
#include "Math/EngineMath.h"
#include <algorithm>

float SPlane::GetSignedDistance(glm::vec3 point)
{
	return glm::dot(Normal, point - Point);
}

bool SPlane::IsSphereOnPlane(vec3 sphereLoc, float radius)
{
	return -radius < GetSignedDistance(sphereLoc);
}

bool SFrustum::IsSphereInFrustum(const SBounds& bounds, const mat4& transform)
{
	const vec3 scale(glm::length(glm::vec3(transform[0])), glm::length(glm::vec3(transform[1])), glm::length(glm::vec3(transform[2])));
	// const float maxScale = std::max({ glm::abs(scale.x), glm::abs(scale.y), glm::abs(scale.z) });
	const float maxScale = std::max({ scale.x, scale.y, scale.z });
	const vec3 worldPos = transform * glm::vec4(bounds.Origin, 1.f);
	const float worldRadius = maxScale * bounds.Radius; // *0.5f;
	const bool bResult = std::all_of(Planes.begin(), Planes.end(), [&](SPlane& p)
	{
		return p.IsSphereOnPlane(worldPos, worldRadius);
	});
	return bResult;
}

void SGlCamera::CalcViewMatrix(glm::mat4& outMat) const
{
	const glm::vec3 front = glm::rotateByQuat(World::Front, Rotation);
	const glm::vec3 up = glm::rotateByQuat(World::Up, Rotation);
    outMat = glm::lookAtLH(Position, Position + front, up);
}

void SGlCamera::CalcProjMatrix(glm::mat4& outMat) const
{
	const float aspectRatio = CEngine::Get()->Viewport.AspectRatio;
	outMat = glm::perspectiveLH(FOV, aspectRatio, NearPlane, FarPlane);
}

void SGlCamera::CalcFrustum(SFrustum& outFrustum) const
{    
	const vec3 front = glm::rotateByQuat(World::Front, Rotation);
	const vec3 up = glm::rotateByQuat(World::Up, Rotation);
	const vec3 right = glm::rotateByQuat(World::Right, Rotation);

    const float halfFarHeight = FarPlane * tanf(FOV * .5f); // tan(FOV) = opposite / adjacent = halfVSide / FarPlane
    const float halfFarWidth = halfFarHeight * CEngine::Get()->Viewport.AspectRatio;
    const vec3 frontMultFar = FarPlane * front;

    outFrustum.Far = { -front, Position + frontMultFar };
    outFrustum.Left = { glm::cross(up, frontMultFar - (right * halfFarWidth)), Position };
    outFrustum.Right = { glm::cross(frontMultFar + (right * halfFarWidth), up), Position };
	outFrustum.Bottom = { glm::cross(-right, frontMultFar - (up * halfFarHeight)), Position };
	outFrustum.Top = { glm::cross(right, frontMultFar + (up * halfFarHeight)), Position };
	std::for_each(outFrustum.Planes.begin(), outFrustum.Planes.end(), [&](auto& p) { p.Normal = glm::normalize(p.Normal); });
}

void SGlCamera::UpdateSceneData(SSceneData& sceneData)
{
	sceneData.CameraPos = glm::vec4(Position, 1.0f);
	CalcViewMatrix(sceneData.View);
	CalcProjMatrix(sceneData.Proj);
	sceneData.ViewProj = sceneData.Proj * sceneData.View; // * Model * V
}

void SGlCamera::UpdateCameraFromInput(GLFWwindow* window, float deltax, float deltay, float deltaTime)
{
	const glm::vec3 front = glm::rotateByQuat(World::Front, Rotation);
	const glm::vec3 up = glm::rotateByQuat(World::Up, Rotation);
	const glm::vec3 right = glm::rotateByQuat(World::Right, Rotation);
	vec3 moveDir(0.0f);
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		moveDir += front;
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		moveDir -= front;
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		moveDir -= right;
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		moveDir += right; 

	vec3 positionDelta(abs(glm::dot(moveDir, moveDir)) > 1e-8f ?
		(Speed * deltaTime * glm::normalize(moveDir)) :
		vec3(0.0f));

	moveDir = vec3(0.0f);
	if (glfwGetKey(window, GLFW_KEY_E))
		moveDir += up;
	if (glfwGetKey(window, GLFW_KEY_Q))
		moveDir -= up;

	positionDelta += abs(glm::dot(moveDir, moveDir)) > 1e-8f ?
		(Speed * deltaTime * glm::normalize(moveDir)) :
		vec3(0.0f);

	Position += positionDelta;
	Yaw += deltax * deltaTime * YawSens;
	Pitch += deltay * deltaTime * PitchSens;
	constexpr float minPitch = glm::radians(-89.f);
	constexpr float maxPitch = glm::radians(89.f);
	Pitch = glm::clamp(Pitch, minPitch, maxPitch);
	Rotation = glm::fromYawPitchRoll(glm::vec3(Yaw, Pitch, 0.0f));
	// std::cout << glm::degrees(angles.x) << " " << glm::degrees(angles.y) << " " << glm::degrees(angles.z) << std::endl;
}
