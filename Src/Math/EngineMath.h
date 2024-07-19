#pragma once

#include <numbers>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/random.hpp>
#include <optional>

#define GLM_ENABLE_EXPERIMENTAL
#define GLM_GTX_norm

#ifdef GLM_GTX_norm
#include <glm/gtx/norm.hpp>
#endif

using glm::vec2;
using glm::vec3;
using glm::vec4;
using glm::mat4;
using glm::quat;

constexpr float pi = glm::pi<float>();
constexpr float SMALLER_NUMBER = 1.e-8f;
constexpr float SMALL_NUMBER = 1.e-4f;

namespace World
{
	constexpr glm::vec3 Front = glm::vec3(0.0f, 0.0f, 1.0f);
	constexpr glm::vec3 Up = glm::vec3(0.0f, 1.0f, 0.0f);
	constexpr glm::vec3 Right = glm::vec3(1.0f, 0.0f, 0.0f);
	constexpr glm::vec3 X = Right;
	constexpr glm::vec3 Y = Up;
	constexpr glm::vec3 Z = Front;
};

namespace glm
{
	constexpr mat4 IMat(1.0f);
	constexpr quat IQuat(1.0f, 0.0f, 0.0f, 0.0f);

	inline vec3 rotateByQuat(const vec3& v, const quat& q) { return q * v; };
	inline vec3 rotateByQuatInverse(const vec3& v, const quat& q) { return v * q; };
	quat fromYawPitchRoll(const vec3& yawPitchRoll); // Constructs quat applying yaw->pitch->roll
	quat fromPitchYawRoll(const vec3& yawPitchRoll); // Constructs quat applying pitch->yaw->roll
}

// #define GLM_ENABLE_EXPERIMENTAL
// #define GLM_GTX_matrix_decompose

struct STransform
{
	STransform();
	STransform(const vec3& pos, const quat& rot, const vec3& scale);
	STransform(const vec3& pos, const vec3& angles, const vec3& scale);
	STransform(const STransform&) = default;

#ifdef GLM_GTX_matrix_decompose
	STransform(const mat4& transformMatrix);
#endif

	mat4 GetMatrix() const; // Gets the transformation matrix Tf = Translation x Rotation x Scale

	inline const vec3& GetPosition() const { return Position; }
	inline const vec3& GetScale() const { return Scale; }
	inline const quat& GetRotation() const { return Rotation; }
	inline const std::optional<vec3>& GetAngles() const { return Angles; }
	inline void SetPosition(const vec3& position) { Position = position; }
	inline void SetScale(const vec3& scale) { Scale = scale; }

	/* Rotation stuff */

	// Assumes a valid rotation, normalized quaternion. Does not store angles.
	inline void SetRotation(const quat& rotation) { Rotation = rotation; Angles.reset(); } 

	// Use if you want to set rotation with angles in an order that isnt yaw->pitch->roll
	inline void SetRotationWithAngles(const quat& rotation, const vec3& angles) { Rotation = rotation; Angles = angles; } 

	// Applies Yaw->Pitch->Roll rotation.
	void SetRotation(const vec3& yawPitchRoll); 

	// Applies this transform to vector v.
	vec3 TransformLocation(const vec3& v) const; 
	// Applies this transform (rotation + translation only) to vector v 
	vec3 TransformDirection(const vec3& v) const; 

	/*
	* Applies the inverse of this transform to location v
	* i.e. If this is a local-to-world transform, this would transform location v from worldspace to localspace.
	*/
	vec3 InverseTransformLocation(const vec3& v);

	/*
	* Applies the inverse of this transform (without scale) to direction v 
	* i.e. if this is a local-to-world transform, this would transform direction v from world-space to local-space.
	*/
	vec3 InverseTransformDirection(const vec3& v);

	// Gets the inverse of this transform
	STransform Inverse() const;

	// Composes transformations
	friend STransform operator*(const STransform& A, const STransform& B);
	inline static STransform ComposeTransforms(const STransform& A, const STransform& B) { return A * B; };

protected:
	quat Rotation;
	std::optional<vec3> Angles;
	vec3 Position;
	vec3 Scale;
};
