#pragma once

#include "RCube/Core/Accel/AABB.h"
#include "RCube/Core/Accel/Ray.h"
#include "glm/glm.hpp"
#include <memory>

namespace rcube
{
class Primitive
{
  public:
    virtual glm::vec3 position() const = 0;
    virtual AABB aabb() const = 0;
    virtual bool rayIntersect(const Ray &ray, float &t) const = 0;
    virtual size_t id() const = 0;
};

using PrimitivePtr = std::shared_ptr<Primitive>;

class Point : public Primitive
{
    glm::vec3 pos_;
    size_t id_;
    float radius_ = 1.f;
    float radius_sq_ = 1.f;

  public:
    Point(size_t id, const glm::vec3 &pos, float radius);

    virtual size_t id() const override;

    virtual bool rayIntersect(const Ray &ray, float &t) const override;

    virtual glm::vec3 position() const override;

    virtual AABB aabb() const override;
};

class Triangle : public Primitive
{
    glm::vec3 v0_, v1_, v2_;
    size_t id_;

  public:
    Triangle(size_t id, glm::vec3 v0, glm::vec3 v1, glm::vec3 v2);

    virtual size_t id() const override;

    virtual bool rayIntersect(const Ray &ray, float &t) const override;

    virtual glm::vec3 position() const;

    AABB aabb() const override;

    glm::vec3 barycentricCoordinate(const glm::vec3 &point) const;

    float area() const;

    size_t closestVertexIndex(const glm::vec3 &pt, float &dist) const;

    const glm::vec3 &vertex(size_t ind) const;
};

} // namespace rcube