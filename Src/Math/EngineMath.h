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

// TODO: Decide if I'm keeping or removing these. In some places I use vec3, in others I use glm::vec3. Should standarize.
using glm::vec2;
using glm::vec3;
using glm::vec4;
using glm::mat4;
using glm::quat;

constexpr float pi = glm::pi<float>();
constexpr float pi_half = glm::half_pi<float>();
constexpr float SMALLER_NUMBER = 1.e-8f;
constexpr float SMALL_NUMBER = 1.e-4f;

// Left-handed coordinate system
namespace World
{
	constexpr glm::vec3 Front = glm::vec3(0.0f, 0.0f, 1.0f);
	constexpr glm::vec3 Up = glm::vec3(0.0f, 1.0f, 0.0f);
	constexpr glm::vec3 Right = glm::vec3(-1.0f, 0.0f, 0.0f);
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
	explicit STransform(const mat4& transformMatrix);

	mat4 GetMatrix() const; // Gets the transformation matrix Tf = Translation x Rotation x Scale

	inline const vec3& GetPosition() const { return Position; }
	inline const vec3& GetScale() const { return Scale; }
	inline const quat& GetRotation() const { return Rotation; }
	inline void SetPosition(const vec3& position) { Position = position; }
	inline void SetScale(const vec3& scale) { Scale = scale; }

	/* Rotation stuff */

	// Assumes a valid rotation, normalized quaternion. Does not store angles.
	inline void SetRotation(const quat& rotation) { Rotation = rotation; } 

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

	// Composes transformations (result of applying first B, then A). Analogous to multiplying transform matrices.
	friend STransform operator*(const STransform& A, const STransform& B);
	// Composes transformations (result of applying first B, then A). Analogous to multiplying transform matrices.
	inline static STransform ComposeTransforms(const STransform& A, const STransform& B) { return A * B; };

	inline static STransform FromScale(const glm::vec3& scale) { STransform t {}; t.SetScale(scale); return t; }
	inline static STransform FromTranslate(const glm::vec3& translation) { STransform t {}; t.SetPosition(translation); return t; }
	inline static STransform FromRotate(const glm::quat& rot) { STransform t {}; t.SetRotation(rot); return t; }

protected:
	quat Rotation { glm::IQuat };
	vec3 Position { 0.f };
	vec3 Scale { 1.f };
};
