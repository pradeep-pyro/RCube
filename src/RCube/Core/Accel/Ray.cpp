#include "RCube/Core/Accel/Ray.h"

namespace rcube
{
Ray::Ray(const glm::vec3 o, const glm::vec3 d, float tmin, float tmax)
    : origin_(o), tmin_(tmin), tmax_(tmax)
{
    direction_ = glm::normalize(d);
    inv_direction_ = 1.0f / direction_;
}

const glm::vec3 &Ray::origin() const
{
    return origin_;
}

void Ray::setOrigin(const glm::vec3 &origin)
{
    origin_ = origin;
}

const glm::vec3 &Ray::direction() const
{
    return direction_;
}

void Ray::setDirection(glm::vec3 direction)
{
    direction_ = glm::normalize(direction);
    inv_direction_ = 1.0f / direction_;
}

const glm::vec3 &Ray::inverseDirection() const
{
    return inv_direction_;
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