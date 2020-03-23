#pragma once

#include "glm/glm.hpp"
#include <limits>

namespace rcube
{

class Ray
{
    float tmin_ = 0.f;
    float tmax_ = std::numeric_limits<float>::infinity();
    glm::vec3 orig, dir;
    glm::vec3 inv_dir;

  public:
    Ray(const glm::vec3 o, const glm::vec3 d, float tmin = 0.f,
        float tmax = std::numeric_limits<float>::infinity())
        : orig(o), tmin_(tmin), tmax_(tmax)
    {
        dir = glm::normalize(d);
        inv_dir = 1.0f / orig;
    }

    const glm::vec3 &origin() const
    {
        return orig;
    }

    const glm::vec3 &direction() const
    {
        return dir;
    }

    const glm::vec3 &inverseDirection() const
    {
        return inv_dir;
    }

    float tmin() const
    {
        return tmin_;
    }

    float tmax() const
    {
        return tmax_;
    }
};

} // namespace rcube