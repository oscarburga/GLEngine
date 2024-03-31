#pragma once

#include "EngineMath.h"

struct World
{
	// static constexpr vec3 front;
	// static constexpr vec3 up;
	// static constexpr vec3 right;
	static constexpr vec3 front = vec3(0.0f, 0.0f, 1.0f);
	static constexpr vec3 up = vec3(0.0f, 1.0f, 0.0f);
	static constexpr vec3 right = vec3(1.0f, 0.0f, 0.0f);
	static constexpr vec3 X = right;
	static constexpr vec3 Y = up;
	static constexpr vec3 Z = front;
};



