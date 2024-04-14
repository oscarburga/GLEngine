#include "EngineMath.h"

vec3 glm::rotateByQuat(const vec3& v, const quat& q)
{
    const quat vecQuat(0.0f, v.x, v.y, v.z);
    const quat result = q * vecQuat * glm::conjugate(q);
    return vec3(result.x, result.y, result.z);
}

vec3 glm::rotateByQuatActive(const vec3& v, const quat& q)
{
    const quat vecQuat(0.0f, v.x, v.y, v.z);
    const quat result = glm::conjugate(q) * vecQuat * q;
    return vec3(result.x, result.y, result.z);
}
