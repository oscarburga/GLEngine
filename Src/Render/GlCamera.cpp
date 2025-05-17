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
    outMat = glm::lookAt(Position, Position + front, up);
}

void SGlCamera::CalcProjMatrix(glm::mat4& outMat) const
{
	const SViewport& viewport = CEngine::Get()->Viewport;
	outMat = bIsPerspective ?
		glm::perspective(PerspectiveFOV, viewport.AspectRatio, NearPlane, FarPlane) :
		glm::ortho(OrthoMinBounds.x, OrthoMaxBounds.x, OrthoMinBounds.y, OrthoMaxBounds.y, NearPlane, FarPlane);
}

void SGlCamera::CalcFrustum(SFrustum* outFrustum, std::array<vec3, 8>* outCorners) const
{    
	if (!outFrustum && !outCorners)
		return;

	const vec3 front = glm::rotateByQuat(World::Front, Rotation);
	const vec3 frontNear = NearPlane * front;
	const vec3 frontFar = FarPlane * front;
	const vec3 up = glm::rotateByQuat(World::Up, Rotation);
	const vec3 right = glm::rotateByQuat(World::Right, Rotation);
	vec2 perspFrustumSizes[2]; // near, far
	vec2 orthoCornersOffset[4];
	if (bIsPerspective)
	{
		const float aspectRatio = CEngine::Get()->Viewport.AspectRatio;
		const float tanFov = glm::tan(PerspectiveFOV * 0.5f);
		const float halfNearHeight = NearPlane * tanFov; // tan(FOV) = opposite / adjacent = halfVSide / FarPlane
		const float halfFarHeight = FarPlane * tanFov; // tan(FOV) = opposite / adjacent = halfVSide / FarPlane
		// const float halfFarWidth = halfFarHeight * CEngine::Get()->Viewport.AspectRatio;
		perspFrustumSizes[0] = { halfNearHeight * aspectRatio , halfNearHeight };
		perspFrustumSizes[1] = { halfFarHeight * aspectRatio, halfFarHeight };
	}
	else 
	{
		orthoCornersOffset[0] = OrthoMinBounds;
		orthoCornersOffset[1] = { OrthoMaxBounds.x, OrthoMinBounds.y };
		orthoCornersOffset[2] = OrthoMaxBounds;
		orthoCornersOffset[3] = { OrthoMinBounds.x, OrthoMaxBounds.y };
	}

	// Corners
	// near { botleft, botright, topright, topleft }, far { botleft, botright, topright, topleft }
	if (outCorners)
	{
		constexpr float dx[] = { -1, 1, 1, -1 };
		constexpr float dy[] = { -1, -1, 1, 1 };
		for (int far = 0; far <= 1; ++far)
		{
			const vec3 faceCenter = Position + (far ? frontFar : frontNear);
			for (int corner = 0; corner < 4; corner++)
			{
				const int cornerIdx = (far * 4) + corner;
				if (bIsPerspective)
				{
					const vec3 xOffset = right * (perspFrustumSizes[far].x * dx[corner]);
					const vec3 yOffset = up * (perspFrustumSizes[far].y * dy[corner]);
					outCorners->at(cornerIdx) = faceCenter + xOffset + yOffset;
				}
				else
				{
					const vec3 xOffset = right * vec3(orthoCornersOffset[corner], 0.f);
					const vec3 yOffset = up * vec3(orthoCornersOffset[corner], 0.f);
					outCorners->at(cornerIdx) = faceCenter + xOffset + yOffset;
				}
			}
		}
	}

	if (outFrustum)
	{

		outFrustum->Near = { front, Position + frontNear };
		outFrustum->Far = { -front, Position + frontFar };

		if (bIsPerspective)
		{
			outFrustum->Left = { glm::cross(frontFar - (right * perspFrustumSizes[1].x), up), Position};
			outFrustum->Right = { glm::cross(up, frontFar + (right * perspFrustumSizes[1].x)), Position};
			outFrustum->Bottom = { glm::cross(right, frontFar - (up * perspFrustumSizes[1].y)), Position};
			outFrustum->Top = { glm::cross(frontFar + (up * perspFrustumSizes[1].y), right), Position};
			std::for_each(outFrustum->Planes.begin(), outFrustum->Planes.end(), [&](auto& p) 
			{ 
				p.Normal = glm::normalize(p.Normal); 
			});
		}
		else
		{
			outFrustum->Left = { right, outFrustum->Near.Point + (right * OrthoMinBounds.x) };
			outFrustum->Right = { -right, outFrustum->Near.Point + (right * OrthoMaxBounds.x) };
			outFrustum->Bottom = { up, outFrustum->Near.Point + (up * OrthoMinBounds.y) };
			outFrustum->Top = { -up, outFrustum->Near.Point + (up * OrthoMaxBounds.y) };
		}
	}
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

	vec3 positionDelta = vec3{ glm::length2(moveDir) > 1e-8f ?
		(Speed * deltaTime * glm::normalize(moveDir)) :
		vec3(0.0f)
	};

	moveDir = vec3(0.0f);
	if (glfwGetKey(window, GLFW_KEY_E))
		moveDir += up;
	if (glfwGetKey(window, GLFW_KEY_Q))
		moveDir -= up;

	positionDelta += glm::length2(moveDir) > 1e-8f ?
		(Speed * deltaTime * glm::normalize(moveDir)) :
		vec3(0.0f);

	Position += positionDelta;
	Yaw -= deltax * deltaTime * YawSens;
	Pitch += deltay * deltaTime * PitchSens;
	constexpr float minPitch = glm::radians(-89.f);
	constexpr float maxPitch = glm::radians(89.f);
	Pitch = glm::clamp(Pitch, minPitch, maxPitch);
	Rotation = glm::fromYawPitchRoll(glm::vec3(Yaw, Pitch, 0.0f));
	std::cout << glm::degrees(Yaw) << " " << glm::degrees(Pitch) << " " << std::endl;
}
