#pragma once

#include "RCube/Core/Accel/Ray.h"
#include "glm/glm.hpp"
#include <algorithm>
#include <array>

namespace rcube
{

class AABB
{
    glm::vec3 min_ = glm::vec3(1.f, 1.f, 1.f);
    glm::vec3 max_ = glm::vec3(-1.f, -1.f, -1.f);

  public:
    AABB() = default;

    AABB(const glm::vec3 &bb_min, const glm::vec3 &bb_max);

    const glm::vec3 &min() const;

    const glm::vec3 &max() const;

    bool isNull() const;

    void setNull();

    void expandBy(const glm::vec3 &p);

    void expandBy(const AABB &other);

    size_t longestAxis() const;

    glm::vec3 size() const;

    bool rayIntersect(const Ray &ray, float &t);

    std::array<glm::vec3, 8> corners() const;

    glm::vec3 center() const;

    float diagonal() const;

    glm::vec3 extents() const;
};

AABB operator*(const glm::mat4 &mat, const AABB &box);

} // namespace rcube