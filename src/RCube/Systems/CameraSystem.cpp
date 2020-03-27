#include "RCube/Systems/CameraSystem.h"
#include "RCube/Components/Camera.h"
#include "RCube/Components/Transform.h"
#include "glm/gtx/string_cast.hpp"

namespace rcube
{

CameraSystem::CameraSystem()
{
    ComponentMask camera_filter;
    camera_filter.set(Transform::family());
    camera_filter.set(Camera::family());
    addFilter(camera_filter);
}

unsigned int CameraSystem::priority() const
{
    return 200;
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
                                                 half_ortho_h, cam->near_plane, cam->far_plane);
        }
        else
        {
            float fH = std::tan(cam->fov) * cam->near_plane;
            float fW = fH * aspect_ratio;
            cam->view_to_projection =
                glm::frustum(-fW, fW, -fH, fH, cam->near_plane, cam->far_plane);
        }
    }
}

} // namespace rcube
