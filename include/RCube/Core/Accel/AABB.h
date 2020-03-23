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
    AABB(const glm::vec3 &bb_min, const glm::vec3 &bb_max) : min_(bb_min), max_(bb_max)
    {
    }
    const glm::vec3 &min() const
    {
        return min_;
    }
    const glm::vec3 &max() const
    {
        return max_;
    }
    bool isNull() const
    {
        return min_.x > max_.x || min_.y > max_.y || min_.z > max_.z;
    }
    void expandBy(const glm::vec3 &p)
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
    void expandBy(const AABB &other)
    {
        expandBy(other.min_);
        expandBy(other.max_);
    }
    size_t longestAxis() const
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
    bool rayIntersect(const Ray &ray)
    {
        float nearT = -std::numeric_limits<float>::infinity();
        float farT = std::numeric_limits<float>::infinity();

        for (int i = 0; i < 3; i++)
        {
            float origin = ray.origin()[i];
            float minVal = min_[i], maxVal = max_[i];

            if (ray.direction()[i] == 0)
            {
                if (origin < minVal || origin > maxVal)
                {
                    return false;
                }
            }
            else
            {
                float t1 = (minVal - origin) / ray.direction()[i];
                float t2 = (maxVal - origin) / ray.direction()[i];

                if (t1 > t2)
                {
                    std::swap(t1, t2);
                }

                nearT = std::max(t1, nearT);
                farT = std::min(t2, farT);

                if (!(nearT <= farT))
                {
                    return false;
                }
            }
        }

        return ray.tmin() <= farT && nearT <= ray.tmax();
    }
};

} // namespace rcube