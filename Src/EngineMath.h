#pragma once

#include <numbers>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/random.hpp>
#include <optional>


using glm::vec2;
using glm::vec3;
using glm::vec4;
using glm::mat4;
using glm::quat;

constexpr float pi = glm::pi<float>();
constexpr float SMALLER_NUMBER = 1.e-8f;
constexpr float SMALL_NUMBER = 1.e-4f;

/*
* https://danceswithcode.net/engineeringnotes/quaternions/quaternions.html
*/
namespace glm
{
	// applies passive quaternion rotation q * v * q-1 (coordinate system is rotated with respect to the point, point remains fixed).
	vec3 rotateByQuat(const vec3& v, const quat& q);
	// applies active quaternion rotation q-1 * v * q (point is rotated with respect to the axes, axes remain fixed).
	vec3 rotateByQuatActive(const vec3& v, const quat& q);

	 quat fromYaw(float Yaw);
	 quat fromPitch(float Pitch);
	 quat fromYawPitch(float Yaw, float Pitch);
	 quat fromPitchYaw(float Pitch, float Yaw);
	 quat fromRoll(float Roll);
	 quat fromYawRoll(float Yaw, float Roll);
	 quat fromRollYaw(float Roll, float Yaw);
	 quat fromPitchRoll(float Pitch, float Roll);
	 quat fromRollPitch(float Roll, float Pitch);
	 quat fromYawPitchRoll(float Yaw, float Pitch, float Roll);
	 inline quat fromYawPitchRoll(const vec3& yawPitchRoll) { return fromYawPitchRoll(yawPitchRoll.x, yawPitchRoll.y, yawPitchRoll.z); }
	 quat fromYawRollPitch(float Yaw, float Roll, float Pitch);
	 inline quat fromYawRollPitch(const vec3& yawRollPitch) { return fromYawRollPitch(yawRollPitch.x, yawRollPitch.y, yawRollPitch.z); }
	 quat fromPitchYawRoll(float Pitch, float Yaw, float Roll);
	 inline quat fromPitchYawRoll(const vec3& pitchYawRoll) { return fromPitchYawRoll(pitchYawRoll.x, pitchYawRoll.y, pitchYawRoll.z); }
	 quat fromPitchRollYaw(float Pitch, float Roll, float Yaw);
	 inline quat fromPitchRollYaw(const vec3& pitchRollYaw) { return fromPitchRollYaw(pitchRollYaw.x, pitchRollYaw.y, pitchRollYaw.z); }
	 quat fromRollYawPitch(float Roll, float Yaw, float Pitch);
	 inline quat fromRollYawPitch(const vec3& rollYawPitch) { return fromRollYawPitch(rollYawPitch.x, rollYawPitch.y, rollYawPitch.z); }
	 quat fromRollPitchYaw(float Roll, float Pitch, float Yaw);
	 inline quat fromRollPitchYaw(const vec3& rollPitchYaw) { return fromRollPitchYaw(rollPitchYaw.x, rollPitchYaw.y, rollPitchYaw.z); }

	// vec3 toYawPitchRoll(const quat& q); // I cannot get this to work for lord knows what reason. Won't support converting from quat to euler lol.
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
	inline void SetRotation(const quat& rotation) { Rotation = rotation; Angles.reset(); } // Assumes a valid rotation, normalized quaternion.
	void SetRotation(const vec3& yawPitchRoll); // Consider using glm::angleAxis and multiplying quaternions instead, potentially better precision.

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
