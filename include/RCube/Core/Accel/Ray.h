#pragma once

#include "glm/glm.hpp"
#include <limits>

namespace rcube
{

class Ray
{
    float tmin_ = 0.f;
    float tmax_ = std::numeric_limits<float>::infinity();
    glm::vec3 origin_, direction_;
    glm::vec3 inv_direction_;

  public:
    Ray() = default;

    Ray(const glm::vec3 o, const glm::vec3 d, float tmin = 0.f,
        float tmax = std::numeric_limits<float>::infinity());

    const glm::vec3 &origin() const;

    void setOrigin(glm::vec3 &origin);

    const glm::vec3 &direction() const;

    void setDirection(glm::vec3 direction);

    const glm::vec3 &inverseDirection() const;

    float tmin() const;

    float tmax() const;
};

} // namespace rcube