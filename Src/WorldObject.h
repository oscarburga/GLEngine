#pragma once

#include "World.h"
#include "EngineMath.h"
#include <vector>
/*
* Base world object class that handles the geometric fundamentals. 
* All game objects with a physical presence (location, orientation) should inherit from this class.
* TODO: Implement scene graph hierarchy
*/
class GWorldObject
{
	/* Members */
public:
	STransform Transform;
protected:
	// STransform RelativeTransform;
	// STransform WorldTransform;
	GWorldObject* Parent = nullptr;
	float CreateTime = 0.0f;

public:
	/* Methods */
	GWorldObject(GWorldObject* Parent = nullptr);
	inline vec3 GetFrontVector() const { return Transform.GetRotation() * (World::Front); }
	inline vec3 GetUpVector() const { return Transform.GetRotation() * (World::Up); }
	inline vec3 GetRightVector() const { return Transform.GetRotation() * (World::Right); }
	// inline vec3 GetFrontVector() const { return WorldTransform.RotateVector(World::Front); }
	// inline vec3 GetUpVector() const { return WorldTransform.RotateVector(World::Up); }
	// inline vec3 GetRightVector() const { return WorldTransform.RotateVector(World::Right); }
};
