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
     * Computes ray-mesh intersection. The Mesh should have a valid BVH computed with Mesh::updateBVH()
     *
     * @param[in] mesh Mesh object
     * @param[in] model_to_world 4x4 matrix to map mesh data to world space
     * @param[in,out] intersect Intersection result
     */
    bool intersect(Mesh *mesh, const glm::mat4 &model_to_world, Intersection &intersect);

    /**
     * Computes ray-mesh intersection with the given entity handles.
     * The entities are expected to have the Drawable, Transform components
     *
     * @param[in] entities List of entity handles to intersect
     * @param[in,out] intersections Intersection results sorted by distance
     */
    void intersect(const std::vector<EntityHandle> &entities, std::vector<Intersection> &intersections);

    /**
     * Computes ray-mesh intersection with the given entities.
     * The entities are expected to have the Drawable, Transform components
     *
     * @param[in] entities List of entities to intersect
     * @param[in] world World where the entities live
     * @param[in,out] intersections Intersection results sorted by distance
     */
    void intersect(const std::vector<Entity> &entities, World *world,
                   std::vector<Intersection> &intersections);
};

} // namespace rcube