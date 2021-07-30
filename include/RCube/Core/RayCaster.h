#pragma once

#include "RCube/Components/Camera.h"
#include "RCube/Components/Transform.h"
#include "RCube/Core/Graphics/OpenGL/Mesh.h"
#include "RCube/Components/Drawable.h"
#include <vector>

namespace rcube
{

/**
 * Holds information about a ray-primitive intersection
 */
struct Intersection
{
    float distance;
    glm::vec3 point;
    PrimitivePtr primitive;
    EntityHandle entity;
};

/**
 * RayCaster is used for shooting rays and finding intersections with objects that have
 * a bounding volume hierarchy.
 */
class RayCaster
{
    Ray ray_; // Ray used for intersection

  public:
    /**
     * Sort the intersections by distance
     * 
     * @param[in,out] intersections Vector of Intersection objects that are sorted in-place
     */
    static void sortByDistance(std::vector<Intersection> &intersections);
    /**
     * Updates the intersection ray based on mouse coordinates and the camera
     * 
     * @param[in] mouse_screen 2D mouse coordinates in screen space
     * @param[in] camera Camera component of the camera used for ray casting
     * @param[in] camera_transform Transform component of the camera used for ray casting
     */
    void setRayFromCamera(const glm::vec2 &mouse_screen, Camera *camera,
                          Transform *camera_transform);

    /**
     * Updates the intersection ray with the given one (in world space)
     * 
     * @param[in] ray_world_space Ray in world space
     */
    void setRay(const Ray &ray_world_space);

    /**
     * Computes ray-BVH intersection.
     *
     * @param[in] bvh Bounding Volume Hierarchy
     * @param[in] model_to_world 4x4 matrix to the BVH data to world space
     * @param[in,out] intersect Intersection result
     */
    bool intersect(BVHNodePtr bvh, const glm::mat4 &model_to_world, Intersection &intersect);
};

} // namespace rcube