#include "RCube/Core/RayCaster.h"
#include <algorithm>

namespace rcube
{

glm::vec2 screenToNDC(double xpos, double ypos, double width, double height)
{
    const double x = xpos;
    const double y = ypos;
    const double w = width;
    const double h = height;
    double ndc_x = (2.0f * x) / w - 1.0f;
    double ndc_y = 1.0f - 2.0f * y / h;
    return glm::vec2(static_cast<float>(ndc_x), static_cast<float>(ndc_y));
}

void sortByDistance(std::vector<Intersection> &intersections)
{
    // Sort intersection results based on distance
    std::sort(intersections.begin(), intersections.end(),
              [](const Intersection &lhs, const Intersection &rhs) {
                  return lhs.distance < rhs.distance;
              });
}

void RayCaster::setRayFromCamera(const glm::vec2 &mouse, Camera *camera,
                                 Transform *camera_transform)
{
    double width = camera->viewport_size[0];
    double height = camera->viewport_size[1];

    // Generate a ray from camera for picking
    const glm::vec2 ndc = screenToNDC(mouse[0], mouse[1], width, height);
    const glm::vec4 ray_clip(ndc.x, ndc.y, -1.0, 1.0);
    glm::vec4 ray_eye = glm::inverse(camera->viewToProjection()) * ray_clip;
    ray_eye.z = -1.f;
    ray_eye.w = 0.f;
    glm::vec3 ray_wor(glm::inverse(camera->worldToView()) * ray_eye);
    ray_.setDirection(glm::normalize(ray_wor));
    ray_.setOrigin(camera_transform->worldPosition());
}

void RayCaster::setRay(const Ray &ray_world_space)
{
    ray_ = ray_world_space;
}

bool RayCaster::intersect(Mesh *mesh, const glm::mat4 &model_to_world, Intersection &intersect_result)
{
    const glm::mat4 model_inv = glm::inverse(model_to_world);
    glm::vec3 ray_origin_model = glm::vec3(model_inv * glm::vec4(ray_.origin(), 1.0));
    glm::vec3 ray_dir_model = glm::normalize(model_inv * glm::vec4(ray_.direction(), 0.0));
    Ray ray_model(ray_origin_model, ray_dir_model);
    glm::vec3 pt;
    PrimitivePtr prim;
    if (mesh->rayIntersect(ray_model, pt, prim))
    {
        intersect_result.distance = glm::length(pt - ray_model.origin());
        intersect_result.point = pt;
        intersect_result.primitive = prim;
        return true;
    }
    return false;
}

void RayCaster::intersect(const std::vector<EntityHandle> &entities,
                          std::vector<Intersection> &intersections)
{
    // Find intersection among all entities that have Drawable and Transform components
    for (EntityHandle ent : entities)
    {
        if (!ent.valid())
        {
            continue;
        }
        Drawable *dr = ent.world->getComponent<Drawable>(ent.entity);
        Transform *tr = ent.world->getComponent<Transform>(ent.entity);

        Intersection intersect_result;
        if (intersect(dr->mesh.get(), tr->worldTransform(), intersect_result))
        {
            intersect_result.entity = ent;
            intersections.push_back(intersect_result);
        }
    }

    // Sort intersection results based on distance
    sortByDistance(intersections);
}


void RayCaster::intersect(const std::vector<Entity> &entities, World *world,
                          std::vector<Intersection> &intersections)
{
    assert(world != nullptr);
    // Find intersection among all entities that have Drawable and Transform components
    for (Entity ent : entities)
    {
        Drawable *dr = world->getComponent<Drawable>(ent);
        Transform *tr = world->getComponent<Transform>(ent);

        Intersection intersect_result;
        if (intersect(dr->mesh.get(), tr->worldTransform(), intersect_result))
        {
            intersect_result.entity = EntityHandle{ent, world};
            intersections.push_back(intersect_result);
        }
    }

    // Sort intersection results based on distance
    sortByDistance(intersections);
}

} // namespace rcube