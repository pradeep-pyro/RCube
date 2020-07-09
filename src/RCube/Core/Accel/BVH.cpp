#include "RCube/Core/Accel/BVH.h"

namespace rcube
{

bool BVHNode::rayIntersect(const Ray &ray, glm::vec3 &pt, PrimitivePtr &primitive)
{
    if (aabb.rayIntersect(ray))
    {
        if ((left != nullptr && left->primitives.size() > 0) || (right != nullptr && right->primitives.size() > 0))
        {
            bool hitLeft = left->rayIntersect(ray, pt, primitive);
            bool hitRight = right->rayIntersect(ray, pt, primitive);
            return hitLeft || hitRight;
        }
        else
        {
            float t;
            for (int i = 0; i < primitives.size(); i++)
            {
                if (primitives[i]->rayIntersect(ray, t))
                {
                    pt = ray.origin() + t * ray.direction();
                    primitive = primitives[i];
                    return true;
                }
            }
        }
        return false;
    }
    return false;
}

BVHNodePtr buildBVH(std::vector<PrimitivePtr> &prims, size_t depth)
{

    auto node = std::make_shared<BVHNode>();
    node->primitives = prims;
    node->aabb = AABB();
    node->left = nullptr;
    node->right = nullptr;

    // No primitives
    if (prims.size() == 0)
    {
        return node;
    }

    // Single triangle in the mesh
    if (prims.size() == 1)
    {
        node->aabb = prims[0]->aabb();
        return node;
    }

    // Many triangles in the mesh
    node->aabb = prims[0]->aabb();
    for (int i = 1; i < prims.size(); ++i)
    {
        node->aabb.expandBy(prims[i]->aabb());
    }

    // Midpoint of all triangles
    glm::vec3 midpt(0, 0, 0);
    for (int i = 0; i < prims.size(); ++i)
    {
        midpt += (prims[i]->position()) / float(prims.size());
    }
    // midpt /= midpt.size();

    std::vector<PrimitivePtr> left_primitives;
    std::vector<PrimitivePtr> right_primitives;

    // split along midpoint of longest axis
    glm::length_t axis = (glm::length_t)node->aabb.longestAxis();
    for (int i = 0; i < prims.size(); ++i)
    {
        const glm::vec3 &curr_midpt = prims[i]->position();

        // Triangles to the left of midpoint go to left child
        if (curr_midpt[axis] <= midpt[axis])
        {
            left_primitives.push_back(prims[i]);
        }
        // Triangles to the right of midpoint go to right child
        else
        {
            right_primitives.push_back(prims[i]);
        }
    }

    if (left_primitives.size() == 0 && right_primitives.size() > 0)
    {
        left_primitives = right_primitives;
    }

    if (left_primitives.size() > 0 && right_primitives.size() == 0)
    {
        right_primitives = left_primitives;
    }

    if (depth < BVH_MAXDEPTH)
    {
        node->left = buildBVH(left_primitives, depth + 1);
        node->right = buildBVH(right_primitives, depth + 1);
    }
    else
    {
        node->left = std::make_shared<BVHNode>();
        node->right = std::make_shared<BVHNode>();
        node->left->primitives = std::vector<PrimitivePtr>();
        node->right->primitives = std::vector<PrimitivePtr>();
    }
    return node;
}

} // namespace rcube
