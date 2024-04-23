#include "EngineMath.h"
#include "RefIgnore.h"
#include <iostream>
#include <format>

#ifdef GLM_GTX_matrix_decompose
#include <glm/gtx/matrix_decompose.hpp>
#endif

namespace
{
	mat4 IMat(1.0f);
	quat IQuat(1.0f, 0.0f, 0.0f, 0.0f);
}

vec3 glm::rotateByQuat(const vec3& v, const quat& q)
{
	/*
	* Rotation by Quaternion: q * v * q-1
	* http://people.csail.mit.edu/bkph/articles/Quaternions.pdf
	* Can be reduced to:
	* T = 2 * (Q x V)
	* V' = V + w(T) + (Q x T)
	*/
	const quat vecQuat(0.0f, v.x, v.y, v.z);
	const quat result = q * vecQuat * glm::conjugate(q);
	return vec3(result.x, result.y, result.z);
	// const vec3 qAsVec(q.x, q.y, q.z);
	// const vec3 T = 2.0f * glm::cross(qAsVec, v);
	// const vec3 result = v + (q.w * T) + glm::cross(qAsVec, T);
	// return result;
}

vec3 glm::rotateByQuatActive(const vec3& v, const quat& q)
{
	const quat vecQuat(0.0f, v.x, v.y, v.z);
	const quat result = glm::conjugate(q) * vecQuat * q;
	return vec3(result.x, result.y, result.z);
}


quat glm::fromYaw(float Yaw)
{
	const float cosYaw = glm::cos(Yaw * 0.5f);
	const float sinYaw = glm::sin(Yaw * 0.5f);
	return quat(
		cosYaw,
		0.0f,
		sinYaw,
		0.0f
	);
}

quat glm::fromPitch(float Pitch)
{
	const float cosPitch = glm::cos(Pitch * 0.5f);
	const float sinPitch = glm::sin(Pitch * 0.5f);
	return quat(
		cosPitch,
		sinPitch,
		0.0f,
		0.0f
	);
}

quat glm::fromYawPitch(float Yaw, float Pitch)
{
	const float cosYaw = glm::cos(Yaw * 0.5f);
	const float sinYaw = glm::sin(Yaw * 0.5f);
	const float cosPitch = glm::cos(Pitch * 0.5f);
	const float sinPitch = glm::sin(Pitch * 0.5f);
	return quat(
		cosYaw * cosPitch,
		cosYaw * sinPitch,
		sinYaw * cosPitch,
		sinYaw * sinPitch
	);
}

quat glm::fromPitchYaw(float Pitch, float Yaw)
{
	const float cosPitch = glm::cos(Pitch * 0.5f);
	const float sinPitch = glm::sin(Pitch * 0.5f);
	const float cosYaw = glm::cos(Yaw * 0.5f);
	const float sinYaw = glm::sin(Yaw * 0.5f);
	return quat(
		cosPitch * cosYaw,
		sinPitch * cosYaw,
		cosPitch * sinYaw,
		 - sinPitch * sinYaw
	);
}

quat glm::fromRoll(float Roll)
{
	const float cosRoll = glm::cos(Roll * 0.5f);
	const float sinRoll = glm::sin(Roll * 0.5f);
	return quat(
		cosRoll,
		0.0f,
		0.0f,
		sinRoll
	);
}

quat glm::fromYawRoll(float Yaw, float Roll)
{
	const float cosYaw = glm::cos(Yaw * 0.5f);
	const float sinYaw = glm::sin(Yaw * 0.5f);
	const float cosRoll = glm::cos(Roll * 0.5f);
	const float sinRoll = glm::sin(Roll * 0.5f);
	return quat(
		cosYaw * cosRoll,
		 - sinYaw * sinRoll,
		sinYaw * cosRoll,
		cosYaw * sinRoll
	);
}

quat glm::fromRollYaw(float Roll, float Yaw)
{
	const float cosRoll = glm::cos(Roll * 0.5f);
	const float sinRoll = glm::sin(Roll * 0.5f);
	const float cosYaw = glm::cos(Yaw * 0.5f);
	const float sinYaw = glm::sin(Yaw * 0.5f);
	return quat(
		cosRoll * cosYaw,
		sinRoll * sinYaw,
		cosRoll * sinYaw,
		sinRoll * cosYaw
	);
}

quat glm::fromPitchRoll(float Pitch, float Roll)
{
	const float cosPitch = glm::cos(Pitch * 0.5f);
	const float sinPitch = glm::sin(Pitch * 0.5f);
	const float cosRoll = glm::cos(Roll * 0.5f);
	const float sinRoll = glm::sin(Roll * 0.5f);
	return quat(
		cosPitch * cosRoll,
		sinPitch * cosRoll,
		sinPitch * sinRoll,
		cosPitch * sinRoll
	);
}

quat glm::fromRollPitch(float Roll, float Pitch)
{
	const float cosRoll = glm::cos(Roll * 0.5f);
	const float sinRoll = glm::sin(Roll * 0.5f);
	const float cosPitch = glm::cos(Pitch * 0.5f);
	const float sinPitch = glm::sin(Pitch * 0.5f);
	return quat(
		cosRoll * cosPitch,
		cosRoll * sinPitch,
		 - sinRoll * sinPitch,
		sinRoll * cosPitch
	);
}

quat glm::fromYawPitchRoll(float Yaw, float Pitch, float Roll)
{
	const float cosYaw = glm::cos(Yaw * 0.5f);
	const float sinYaw = glm::sin(Yaw * 0.5f);
	const float cosPitch = glm::cos(Pitch * 0.5f);
	const float sinPitch = glm::sin(Pitch * 0.5f);
	const float cosRoll = glm::cos(Roll * 0.5f);
	const float sinRoll = glm::sin(Roll * 0.5f);
	return quat(
		cosYaw * cosPitch * cosRoll - sinYaw * sinPitch * sinRoll,
		cosYaw * sinPitch * cosRoll - sinYaw * cosPitch * sinRoll,
		cosYaw * sinPitch * sinRoll + sinYaw * cosPitch * cosRoll,
		cosYaw * cosPitch * sinRoll + sinYaw * sinPitch * cosRoll
	);
}

quat glm::fromYawRollPitch(float Yaw, float Roll, float Pitch)
{
	const float cosYaw = glm::cos(Yaw * 0.5f);
	const float sinYaw = glm::sin(Yaw * 0.5f);
	const float cosRoll = glm::cos(Roll * 0.5f);
	const float sinRoll = glm::sin(Roll * 0.5f);
	const float cosPitch = glm::cos(Pitch * 0.5f);
	const float sinPitch = glm::sin(Pitch * 0.5f);
	return quat(
		cosYaw * cosRoll * cosPitch - sinYaw *  - sinRoll * sinPitch,
		cosYaw * cosRoll * sinPitch - sinYaw * sinRoll * cosPitch,
		cosYaw *  - sinRoll * sinPitch + sinYaw * cosRoll * cosPitch,
		cosYaw * sinRoll * cosPitch + sinYaw * cosRoll * sinPitch
	);
}

quat glm::fromPitchYawRoll(float Pitch, float Yaw, float Roll)
{
	const float cosPitch = glm::cos(Pitch * 0.5f);
	const float sinPitch = glm::sin(Pitch * 0.5f);
	const float cosYaw = glm::cos(Yaw * 0.5f);
	const float sinYaw = glm::sin(Yaw * 0.5f);
	const float cosRoll = glm::cos(Roll * 0.5f);
	const float sinRoll = glm::sin(Roll * 0.5f);
	return quat(
		cosPitch * cosYaw * cosRoll - sinPitch *  - sinYaw * sinRoll,
		cosPitch *  - sinYaw * sinRoll + sinPitch * cosYaw * cosRoll,
		cosPitch * sinYaw * cosRoll + sinPitch * cosYaw * sinRoll,
		cosPitch * cosYaw * sinRoll - sinPitch * sinYaw * cosRoll
	);
}

quat glm::fromPitchRollYaw(float Pitch, float Roll, float Yaw)
{
	const float cosPitch = glm::cos(Pitch * 0.5f);
	const float sinPitch = glm::sin(Pitch * 0.5f);
	const float cosRoll = glm::cos(Roll * 0.5f);
	const float sinRoll = glm::sin(Roll * 0.5f);
	const float cosYaw = glm::cos(Yaw * 0.5f);
	const float sinYaw = glm::sin(Yaw * 0.5f);
	return quat(
		cosPitch * cosRoll * cosYaw - sinPitch * sinRoll * sinYaw,
		cosPitch * sinRoll * sinYaw + sinPitch * cosRoll * cosYaw,
		cosPitch * cosRoll * sinYaw + sinPitch * sinRoll * cosYaw,
		cosPitch * sinRoll * cosYaw - sinPitch * cosRoll * sinYaw
	);
}

quat glm::fromRollYawPitch(float Roll, float Yaw, float Pitch)
{
	const float cosRoll = glm::cos(Roll * 0.5f);
	const float sinRoll = glm::sin(Roll * 0.5f);
	const float cosYaw = glm::cos(Yaw * 0.5f);
	const float sinYaw = glm::sin(Yaw * 0.5f);
	const float cosPitch = glm::cos(Pitch * 0.5f);
	const float sinPitch = glm::sin(Pitch * 0.5f);
	return quat(
		cosRoll * cosYaw * cosPitch - sinRoll * sinYaw * sinPitch,
		cosRoll * cosYaw * sinPitch + sinRoll * sinYaw * cosPitch,
		cosRoll * sinYaw * cosPitch - sinRoll * cosYaw * sinPitch,
		cosRoll * sinYaw * sinPitch + sinRoll * cosYaw * cosPitch
	);
}

quat glm::fromRollPitchYaw(float Roll, float Pitch, float Yaw)
{
	const float cosRoll = glm::cos(Roll * 0.5f);
	const float sinRoll = glm::sin(Roll * 0.5f);
	const float cosPitch = glm::cos(Pitch * 0.5f);
	const float sinPitch = glm::sin(Pitch * 0.5f);
	const float cosYaw = glm::cos(Yaw * 0.5f);
	const float sinYaw = glm::sin(Yaw * 0.5f);
	return quat(
		cosRoll * cosPitch * cosYaw - sinRoll *  - sinPitch * sinYaw,
		cosRoll * sinPitch * cosYaw + sinRoll * cosPitch * sinYaw,
		cosRoll * cosPitch * sinYaw - sinRoll * sinPitch * cosYaw,
		cosRoll *  - sinPitch * sinYaw + sinRoll * cosPitch * cosYaw
	);
}

STransform::STransform() : Rotation(IQuat), Angles(vec3(0.0f)), Position(0.0f), Scale(1.0f) {}

STransform::STransform(const vec3& pos, const vec3& angles, const vec3& scale) : Angles(angles), Position(pos), Scale(scale) 
{
	Rotation = glm::fromYawPitchRoll(angles);
}

STransform::STransform(const vec3& pos, const quat& rot, const vec3& scale) : Rotation(rot), Angles(std::nullopt), Position(pos), Scale(scale) {}

#ifdef GLM_GTX_matrix_decompose
STransform::STransform(const mat4& transformMatrix)
{
	if (glm::decompose(transformMatrix, Scale, Rotation, Position, RefIgnore<vec3>::I, RefIgnore<vec4>::I))
	{
		Angles = glm::toYawPitchRoll(Rotation);
	} 
	else 
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
		Angles = vec3(0.0f);
	}
}
#endif
// #pragma warning(pop)

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
	Angles = yawPitchRoll; 
	// Consider using glm::angleAxis and multiplying quaternions instead, potentially better precision.
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
	* To transform a vector V by first applying A, then B:
	* V' = Tb x Rb x Sb x Ta x Ra x Sa x V
	*
	* Final Translation: Tb x Rb x Sb x Ta -> B.Position + (B.RotateVector(B.Scale * A.Position))
	* Final Rotation: Rb x Ra = B.Rotation x A.Rotation.
	* Final Scale = Sb x Sa
	*/
	const quat rotation = B.Rotation * A.Rotation;
	const vec3 scale = B.Scale * A.Scale;
	const vec3 translate = B.Position + (B.Rotation * (B.Scale * A.Position));
	return STransform(translate, rotation, scale);
}

