#include "RCube/Core/Accel/Ray.h"

namespace rcube
{
Ray::Ray(const glm::vec3 o, const glm::vec3 d, float tmin, float tmax)
    : orig(o), tmin_(tmin), tmax_(tmax)
{
    dir = glm::normalize(d);
    inv_dir = 1.0f / orig;
}

const glm::vec3 &Ray::origin() const
{
    return orig;
}

const glm::vec3 &Ray::direction() const
{
    return dir;
}

const glm::vec3 &Ray::inverseDirection() const
{
    return inv_dir;
}

float Ray::tmin() const
{
    return tmin_;
}

float Ray::tmax() const
{
    return tmax_;
}

} // namespace rcube