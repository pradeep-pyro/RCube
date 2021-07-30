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

void RayCaster::sortByDistance(std::vector<Intersection> &intersections)
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

bool RayCaster::intersect(BVHNodePtr bvh, const glm::mat4 &model_to_world, Intersection &intersect_result)
{
    const glm::mat4 model_inv = glm::inverse(model_to_world);
    glm::vec3 ray_origin_model = glm::vec3(model_inv * glm::vec4(ray_.origin(), 1.0));
    glm::vec3 ray_dir_model = glm::normalize(model_inv * glm::vec4(ray_.direction(), 0.0));
    Ray ray_model(ray_origin_model, ray_dir_model);
    glm::vec3 pt;
    PrimitivePtr prim;
    if (bvh == nullptr)
    {
        return false;
    }
    BVHClosestIntersectionInfo info;
    bvh->rayClosestIntersect(ray_model, info);
    if (!info.hit)
    {
        return false;
    }
    pt = ray_model.origin() + info.t * ray_model.direction();
    prim = info.primitive;
    intersect_result.distance = glm::length(pt - ray_model.origin());
    intersect_result.point = pt;
    intersect_result.primitive = prim;
    return true;
}

} // namespace rcube