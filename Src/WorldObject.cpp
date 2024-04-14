#include "WorldObject.h"
#include "World.h"

GWorldObject::GWorldObject()
{
    UpdateRotationFromAngles();
}

mat4 GWorldObject::GetTransformMatrix() const
{
    const quat quaternion = GetRotationQuat();
    mat4 transform(1.0f);
    transform[3] = vec4(Position, 1);
    transform *= glm::mat4_cast(quaternion);
    transform = glm::scale(transform, Scale);
    return transform;
}

void GWorldObject::SetRotation(float _pitch, float _yaw, float _roll)
{
    Angles.x = _pitch;
    Angles.y = _yaw;
    Angles.z = _roll;
    UpdateRotationFromAngles();
}

void GWorldObject::UpdateRotationFromAngles()
{
    Quaternion = quat(Angles);
    Front = glm::rotateByQuat(World::front, Quaternion);
    Up = glm::rotateByQuat(World::up, Quaternion);
    Right = glm::cross(Front, Up);
}
