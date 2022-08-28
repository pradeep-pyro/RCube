#include "RCube/Systems/CameraSystem.h"
#include "RCube/Components/Camera.h"
#include "RCube/Components/Transform.h"
#include "glm/gtx/string_cast.hpp"

namespace rcube
{

CameraSystem::CameraSystem()
{
    ComponentMask camera_filter(Transform::family(), Camera::family());
    addFilter(camera_filter);
}

unsigned int CameraSystem::priority() const
{
    return 200;
}

void CameraSystem::fitToExtents(Camera *cam, Transform *tr, const AABB &scene_bounding_box_world_space)
{
    const glm::vec3 center = scene_bounding_box_world_space.center();
    float bounding_size = scene_bounding_box_world_space
                              .size()[glm::length_t(scene_bounding_box_world_space.longestAxis())];
    // Don't try to fit if the bounding box is empty
    if (bounding_size < 1e-5)
    {
        return;
    }
    float approx_radius = -1.f;
    for (const glm::vec3 &corner : scene_bounding_box_world_space.corners())
    {
        approx_radius = std::max(approx_radius, glm::length(center - corner));
    }
    float distance = approx_radius / std::sin(0.5f * cam->fov);
    glm::vec3 forward = glm::normalize(cam->target - tr->position());
    cam->target = center;
    tr->setPosition(center - forward * distance);
}

void CameraSystem::fitNearFarPlanes(Camera *cam, const AABB &scene_bounding_box_world_space)
{
    AABB view_bbox = cam->worldToView() * scene_bounding_box_world_space;
    // Find the closest and farthest z-coordinates to set the near and far planes
    float min_z = std::numeric_limits<float>::infinity();
    float max_z = -std::numeric_limits<float>::infinity();
    for (const glm::vec3 &pt : view_bbox.corners())
    {
        min_z = std::min(min_z, pt.z);
        max_z = std::max(max_z, pt.z);
    }
    cam->near_plane = std::abs(max_z * 0.8f);
    cam->far_plane = std::abs(min_z * 1.2f);
}

void CameraSystem::preUpdate()
{
    for (const Entity &e : registered_entities_[filters_[0]])
    {
        Camera *cam = world_->getComponent<Camera>(e);
        if (cam->needs_fit_to_extents_)
        {
            Transform *tr = world_->getComponent<Transform>(e);
            fitToExtents(cam, tr, cam->fit_to_box_);
        }
    }
}

void CameraSystem::update(bool /* force */)
{
    for (const Entity &e : registered_entities_[filters_[0]])
    {
        Camera *cam = world_->getComponent<Camera>(e);

        const float aspect_ratio = float(cam->viewport_size.x) / float(cam->viewport_size.y);

        const float half_w = 0.5f * cam->viewport_size.x;
        const float half_h = 0.5f * cam->viewport_size.y;
        cam->projection_to_viewport =
            glm::mat4(half_w, 0.f, 0.f, 0.f, 0.f, half_h, 0.f, 0.f, 0.f, 0.f,
                      0.5f * (cam->far_plane - cam->near_plane), 0.f, half_w, half_h,
                      0.5f * (cam->far_plane + cam->near_plane), 1.f);
        Transform *tr = world_->getComponent<Transform>(e);
        cam->world_to_view =
            glm::lookAt(tr->worldPosition(), cam->target, tr->orientation() * YAXIS_POSITIVE);

        if (cam->orthographic)
        {
            float half_ortho_h = cam->orthographic_size / 2.f;
            float half_ortho_w = half_ortho_h * aspect_ratio;
            cam->view_to_projection = glm::ortho(-half_ortho_w, half_ortho_w, -half_ortho_h,
                                                 half_ortho_h, -0.5f * cam->far_plane, 0.5f * cam->far_plane);
        }
        else
        {
            cam->view_to_projection =
                glm::perspective(cam->fov, aspect_ratio, cam->near_plane, cam->far_plane);
        }
    }
}

void CameraSystem::postUpdate()
{
    for (const Entity &e : registered_entities_[filters_[0]])
    {
        Camera *cam = world_->getComponent<Camera>(e);
        if (cam->needs_fit_to_extents_)
        {
            fitNearFarPlanes(cam, cam->fit_to_box_);
            cam->needs_fit_to_extents_ = false;
        }
    }
}

} // namespace rcube
