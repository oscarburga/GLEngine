#pragma once

#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"

struct World
{
	static constexpr glm::vec3 Front = glm::vec3(0.0f, 0.0f, 1.0f);
	static constexpr glm::vec3 Up = glm::vec3(0.0f, 1.0f, 0.0f);
	static constexpr glm::vec3 Right = glm::vec3(1.0f, 0.0f, 0.0f);
	static constexpr glm::vec3 X = Right;
	static constexpr glm::vec3 Y = Up;
	static constexpr glm::vec3 Z = Front;

	// static inline glm::quat YawQuat(const float angle) 
	// {
	// 	float a = angle * 0.5f; 
	// 	return quat(glm::cos(a), 0.0f, glm::sin(a), 0.0f); 
	// }
	// static inline glm::quat PitchQuat(const float angle) 
	// {
	// 	float a = angle * 0.5f; 
	// 	return quat(glm::cos(a), glm::sin(a), 0.0f, 0.0f); 
	// }
	// static inline glm::quat RollQuat(const float angle) 
	// { 
	// 	return glm::angleAxis(angle, Front); 
	// }

};



