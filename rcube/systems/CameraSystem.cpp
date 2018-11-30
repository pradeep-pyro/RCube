#include "CameraSystem.h"
#include "../components/Camera.h"
#include "../components/Transform.h"
#include "glm/gtx/string_cast.hpp"

namespace rcube {

CameraSystem::CameraSystem() {
    ComponentMask camera_filter;
    camera_filter.set(Transform::family());
    camera_filter.set(Camera::family());
    addFilter(camera_filter);
}

void CameraSystem::update(bool /* force */ ) {
    for (const Entity &e : registered_entities_[filters_[0]]) {
        Camera *cam = world_->getComponent<Camera>(e);

        float aspect_ratio = float(cam->viewport_size.x) / float(cam->viewport_size.y);

        float half_w = 0.5f * cam->viewport_size.x;
        float half_h = 0.5f * cam->viewport_size.y;
        cam->projection_to_viewport = glm::mat4(half_w, 0.f, 0.f, 0.f,
                                                0.f, half_h, 0.f, 0.f,
                                                0.f, 0.f, 0.5f * (cam->far - cam->near), 0.f,
                                                half_w, half_h, 0.5f * (cam->far + cam->near), 1.f);
        Transform *tr = world_->getComponent<Transform>(e);
        glm::vec3 pos = tr->worldPosition();
        cam->world_to_view = glm::lookAt(pos, cam->target, cam->up);

        if (cam->orthographic) {
            float half_ortho_h = cam->orthographic_size / 2.f;
            float half_ortho_w = half_ortho_h * aspect_ratio;
            cam->view_to_projection = glm::ortho(-half_ortho_w, half_ortho_w, -half_ortho_h, half_ortho_h,
                                                 cam->near, cam->far);
        }
        else {
            float fH = std::tan(cam->fov) * cam->near;
            float fW = fH * aspect_ratio;
            cam->view_to_projection = glm::frustum(-fW, fW, -fH, fH, cam->near, cam->far);
        }
    }
}

} // namespace rcube
