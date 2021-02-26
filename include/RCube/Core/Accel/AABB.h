#pragma once

#include "RCube/Core/Accel/Ray.h"
#include "glm/glm.hpp"
#include <algorithm>

namespace rcube
{

class AABB
{
    glm::vec3 min_ = glm::vec3(0.f, 0.f, 0.f);
    glm::vec3 max_ = glm::vec3(0.f, 0.f, 0.f);

  public:
    AABB() = default;

    AABB(const glm::vec3 &bb_min, const glm::vec3 &bb_max);

    const glm::vec3 &min() const;

    const glm::vec3 &max() const;

    bool isNull() const;

    void expandBy(const glm::vec3 &p);

    void expandBy(const AABB &other);

    size_t longestAxis() const;

    glm::vec3 size() const;

    bool rayIntersect(const Ray &ray, float &t);
};

} // namespace rcube