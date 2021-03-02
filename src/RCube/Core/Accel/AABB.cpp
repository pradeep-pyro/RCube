#include "RCube/Core/Accel/AABB.h"

namespace rcube
{

AABB::AABB(const glm::vec3 &bb_min, const glm::vec3 &bb_max) : min_(bb_min), max_(bb_max)
{
}

const glm::vec3 &AABB::min() const
{
    return min_;
}

const glm::vec3 &AABB::max() const
{
    return max_;
}

bool AABB::isNull() const
{
    return min_.x > max_.x || min_.y > max_.y || min_.z > max_.z;
}

void AABB::setNull()
{
    min_ = glm::vec3(1.f, 1.f, 1.f);
    max_ = glm::vec3(-1.f, -1.f, -1.f);
}

void AABB::expandBy(const glm::vec3 &p)
{
    if (!isNull())
    {
        min_ = glm::min(p, min_);
        max_ = glm::max(p, max_);
    }
    else
    {
        min_ = p;
        max_ = p;
    }
}

void AABB::expandBy(const AABB &other)
{
    expandBy(other.min_);
    expandBy(other.max_);
}

size_t AABB::longestAxis() const
{
    const glm::vec3 dims = max_ - min_;
    if (dims[0] > dims[1] && dims[0] > dims[2])
    {
        return 0;
    }
    if (dims[1] > dims[0] && dims[1] > dims[2])
    {
        return 1;
    }
    return 2;
}

glm::vec3 AABB::size() const
{
    return glm::abs(max_ - min_);
}

bool AABB::rayIntersect(const Ray &ray, float &t)
{
    float t1 = (min_.x - ray.origin().x) * ray.inverseDirection().x;
    float t2 = (max_.x - ray.origin().x) * ray.inverseDirection().x;
    float t3 = (min_.y - ray.origin().y) * ray.inverseDirection().y;
    float t4 = (max_.y - ray.origin().y) * ray.inverseDirection().y;
    float t5 = (min_.z - ray.origin().z) * ray.inverseDirection().z;
    float t6 = (max_.z - ray.origin().z) * ray.inverseDirection().z;

    float tmin = std::max(std::max(std::min(t1, t2), std::min(t3, t4)), std::min(t5, t6));
    float tmax = std::min(std::min(std::max(t1, t2), std::max(t3, t4)), std::max(t5, t6));

    // AABB is behind rays's origin
    if (tmax < 0)
    {
        t = tmax;
        return false;
    }

    // No intersection
    if (tmin > tmax)
    {
        t = tmax;
        return false;
    }

    t = tmin;
    return true;
}

} // namespace rcube