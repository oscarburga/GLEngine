#include "EngineMath.h"
#include <iostream>
#include <format>

#include "Utils/RefIgnore.h"
#include <glm/gtx/matrix_decompose.hpp>

using glm::IMat;
using glm::IQuat;

quat glm::fromYawPitchRoll(const vec3& yawPitchRoll)
{
	const quat yawQuat = glm::angleAxis(yawPitchRoll.x, World::Up);
	const vec3 right = yawQuat * World::Right;
	const quat pitchQuat = glm::angleAxis(yawPitchRoll.y, right);
	const quat yawPitchQuat = pitchQuat * yawQuat;
	const vec3 front = yawPitchQuat * World::Front;
	const quat rollQuat = glm::angleAxis(yawPitchRoll.z, front);
	return rollQuat * yawPitchQuat;
}

quat glm::fromPitchYawRoll(const vec3& yawPitchRoll)
{
	const quat pitchQuat = glm::angleAxis(yawPitchRoll.y, World::Right);
	const vec3 up = pitchQuat * World::Up;
	const quat yawQuat = glm::angleAxis(yawPitchRoll.x, up);
	const quat pitchYawQuat = yawQuat * pitchQuat;
	const vec3 front = (pitchYawQuat) * World::Front;
	const quat rollQuat = glm::angleAxis(yawPitchRoll.z, front);
	return rollQuat * pitchYawQuat;
}

STransform::STransform() : Rotation(IQuat), Position(0.0f), Scale(1.0f) {}

STransform::STransform(const vec3& pos, const vec3& angles, const vec3& scale) : Position(pos), Scale(scale) 
{
	Rotation = glm::fromYawPitchRoll(angles);
}

STransform::STransform(const vec3& pos, const quat& rot, const vec3& scale) : Rotation(rot), Position(pos), Scale(scale) {}

STransform::STransform(const mat4& transformMatrix)
{
	if (!glm::decompose(transformMatrix, Scale, Rotation, Position, util::RefIgnore<vec3>::I, util::RefIgnore<vec4>::I))
	{
		std::cerr << "Error: Failed to decompose transform matrix:\n";
		// GLM matrices are in column-major format. Print them in row-major.
		for (uint8_t i = 0; i < 4; i++)
		{
			std::cerr << " [";
			for (uint8_t j = 0; j < 4; j++)
			{
				std::cerr << transformMatrix[j][i] << (j == 3) ? "]\n" : ", ";
			}
		}
		std::cerr << "Falling back to an identity transform\n";
		Rotation = IQuat;
		Position = vec3(0.0f);
		Scale = vec3(1.0f);
	}
}

mat4 STransform::GetMatrix() const
{
	mat4 matrix = IMat;
	matrix[3] = vec4(Position, 1);
	// Only apply rotation if the object has a significant rotation. Don't waste time multiplying by an identity matrix.
	if (glm::epsilonNotEqual(Rotation.w, 1.f, SMALL_NUMBER) || abs(Rotation.x) > SMALL_NUMBER || abs(Rotation.y) > SMALL_NUMBER || abs(Rotation.z) > SMALL_NUMBER)
	{
		matrix *= glm::mat4_cast(Rotation);
	}
	matrix = glm::scale(matrix, Scale);
	return matrix;
}

void STransform::SetRotation(const vec3& yawPitchRoll) 
{ 
	Rotation = glm::fromYawPitchRoll(yawPitchRoll);
}

vec3 STransform::TransformLocation(const vec3& v) const
{
	vec3 result = v * Scale;
	result = glm::rotateByQuat(result, Rotation);
	result += Position;
	return result;
}

vec3 STransform::TransformDirection(const vec3& v) const
{
	vec3 result = glm::rotateByQuat(v, Rotation);
	result += Position;
	return result;
}

vec3 STransform::InverseTransformLocation(const vec3& v)
{
	const quat inverseRotation = glm::conjugate(Rotation);
	const vec3 inverseScale = vec3(1.0f) / Scale;
	vec3 result = v - Position;
	result = glm::rotateByQuat(result, inverseRotation);
	result *= inverseScale;
	return result;
}

vec3 STransform::InverseTransformDirection(const vec3& v)
{
	const quat inverseRotation = glm::conjugate(Rotation);
	vec3 result = v - Position;
	result = glm::rotateByQuat(result, inverseRotation);
	return result;
}

STransform STransform::Inverse() const
{
	/*
	* Rotation: Get the inverse quaternion ~R
	* Scale: Get the scale reciprocal ~S
	* Translation: - ~R.Rotate(~S * T)
	*/
	const quat inverseRotation = glm::conjugate(Rotation);
	const vec3 inverseScale = vec3(1.0f) / Scale;
	const vec3 scaledTranslation = inverseScale * Position;
	const vec3 inversePositionNegated = -rotateByQuat(scaledTranslation, inverseRotation);
	return STransform(inversePositionNegated, inverseRotation, inverseScale);
}

STransform operator*(const STransform& A, const STransform& B)
{
	/*
	* To transform a vector V by first applying B, then A:
	* V' = Ta x Ra x Sa x Tb x Rb x Sb x V
	*
	* Final Translation: Ta x Ra x Sa x Tb -> A.Position + (A.RotateVector(A.Scale * B.Position))
	* Final Rotation: Ra x Rb = A.Rotation x B.Rotation.
	* Final Scale = Sa x Sb
	*/
	const quat rotation = A.Rotation * B.Rotation;
	const vec3 scale = A.Scale * B.Scale;
	const vec3 translate = A.Position + glm::rotateByQuat((A.Scale * B.Position), A.Rotation);
	return STransform(translate, rotation, scale);
}

