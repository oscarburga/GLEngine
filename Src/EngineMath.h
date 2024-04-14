#pragma once

#include <numbers>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using glm::vec2;
using glm::vec3;
using glm::vec4;
using glm::mat4;
using glm::quat;
using std::numbers::pi;

namespace glm
{
	// applies passive quaternion rotation (coordinate system is rotated with respect to the point, point remains fixed).
	vec3 rotateByQuat(const vec3& v, const quat& q);
	// applies active quaternion rotation (point is rotated with respect to the axes, axes remain fixed).
	vec3 rotateByQuatActive(const vec3& v, const quat& q);
}

template<typename T>
concept IsNumeric = std::is_integral_v<T>;

