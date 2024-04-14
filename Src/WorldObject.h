#pragma once

#include "EngineMath.h"
/*
* Base world object class that handles the geometric fundamentals. 
* All game objects with a physical presence (location, orientation) should inherit from this class.
*/
class GWorldObject
{
	/* Members */
public:
	GWorldObject();
	vec3 Position = { 0.0f, 0.0f, 0.0f };
	vec3 Scale = { 1.0f, 1.0f, 1.0f };
protected:
	quat Quaternion;
	vec3 Angles = { 0.0f, 0.0f, 0.0f }; // Euler angles in RADIANS (pitch, yaw, roll).
	vec3 Front;
	vec3 Up;
	vec3 Right;

	/* Methods */
public:
	void SetRotation(float _pitch, float _yaw, float _roll);
	void UpdateRotationFromAngles();
	mat4 GetTransformMatrix() const;
	quat GetRotationQuat() const { return Quaternion; }
	vec3 GetFrontVector() const { return Front; }
	vec3 GetUpVector() const { return Up; }
	vec3 GetRightVector() const { return Right; }
	vec3 GetRotation() const { return Angles; }
};
