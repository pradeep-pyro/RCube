#include "RCube/Core/Accel/AABB.h"
#include "RCube/Core/Accel/Primitive.h"
#include "RCube/Core/Accel/Ray.h"
#include "glm/glm.hpp"
#include <memory>
#include <vector>

namespace rcube
{

struct BVHClosestIntersectionInfo
{
    float t = std::numeric_limits<float>::infinity();
    PrimitivePtr primitive;
    bool hit = false;
};

// BVH Tree node
class BVHNode
{
  public:
    AABB aabb;
    std::shared_ptr<BVHNode> left;
    std::shared_ptr<BVHNode> right;
    std::vector<PrimitivePtr> primitives;
    bool rayIntersect(const Ray &ray, glm::vec3 &pt, PrimitivePtr &primitive);
    void rayClosestIntersect(const Ray &ray, BVHClosestIntersectionInfo &info);
};

using BVHNodePtr = std::shared_ptr<BVHNode>;

constexpr size_t BVH_MAXDEPTH = 10;

BVHNodePtr buildBVH(const std::vector<PrimitivePtr> &prims, size_t depth = 0);

} // namespace rcube