#include "RCubeViewer/Systems/PickSystem.h"
#include "RCube/Components/Camera.h"
#include "RCube/Components/Drawable.h"
#include "RCube/Components/Transform.h"
#include "RCubeViewer/Components/CameraController.h"
#include "RCubeViewer/Components/Pickable.h"
#include "RCubeViewer/Pointcloud.h"
#include "imgui.h"

namespace rcube
{
namespace viewer
{

glm::vec2 screenToNDC(double xpos, double ypos, double width, double height)
{
    const float x = static_cast<float>(xpos);
    const float y = static_cast<float>(ypos);
    const float w = static_cast<float>(width);
    const float h = static_cast<float>(height);
    float ndc_x = (2.0f * x) / w - 1.0f;
    float ndc_y = 1.0f - 2.0f * y / h;
    return glm::vec2(ndc_x, ndc_y);
}

PickSystem::PickSystem()
{
    addFilter({Camera::family(), CameraController::family()});
    addFilter({Drawable::family(), Transform::family(), Pickable::family()});
}

void PickSystem::update(bool)
{
    if (ImGui::GetIO().WantCaptureMouse)
    {
        return;
    }
    if (InputState::instance().isMouseJustDown(InputState::Mouse::Left))
    {
        const glm::dvec2 xy = InputState::instance().mousePos();
        // For each camera that is controlled by the user actively (hopefully only 1),
        for (Entity cam_ent : getFilteredEntities({Camera::family(), CameraController::family()}))
        {
            Camera *cam = world_->getComponent<Camera>(cam_ent);
            Transform *cam_tr = world_->getComponent<Transform>(cam_ent);
            double width = cam->viewport_size[0];
            double height = cam->viewport_size[1];

            // ...generate a ray from camera for picking.
            const glm::vec2 ndc = screenToNDC(xy[0], xy[1], width, height);
            const glm::vec4 ray_clip(ndc.x, ndc.y, -1.0, 1.0);
            glm::vec4 ray_eye = glm::inverse(cam->viewToProjection()) * ray_clip;
            ray_eye.z = -1.f;
            ray_eye.w = 0.f;
            glm::vec3 ray_wor(glm::inverse(cam->worldToView()) * ray_eye);
            ray_wor = glm::normalize(ray_wor);

            // Closest hit info
            float min_dist = std::numeric_limits<float>::infinity();
            Entity closest;
            size_t closest_id = 0;
            glm::vec3 closest_point;
            bool hit = false;

            // Find the closest hit among all entities that have Drawable, Transform and Pickable
            // components
            for (Entity ent :
                 getFilteredEntities({Drawable::family(), Transform::family(), Pickable::family()}))
            {
                Drawable *dr = world_->getComponent<Drawable>(ent);
                Transform *tr = world_->getComponent<Transform>(ent);
                Pickable *pickable = world_->getComponent<Pickable>(ent);

                const glm::mat4 model_inv = glm::inverse(tr->worldTransform());
                glm::vec3 ray_origin_model =
                    glm::vec3(model_inv * glm::vec4(cam_tr->worldPosition(), 1.0));
                glm::vec3 ray_dir_model = glm::normalize(model_inv * glm::vec4(ray_wor, 0.0));
                Ray ray_model(ray_origin_model, ray_dir_model);
                glm::vec3 pt;
                size_t id;
                if (dr->mesh->rayIntersect(ray_model, pt, id))
                {
                    hit = true;
                    min_dist = std::min(glm::length(pt - ray_model.origin()), min_dist);
                    closest_point = pt;
                    closest_id = id;
                    closest = ent;
                }
                else
                {
                    pickable->picked = false;
                }
            }
            if (hit)
            {
                Pickable *closest_pick = world_->getComponent<Pickable>(closest);
                Drawable *closest_dr = world_->getComponent<Drawable>(closest);
                closest_pick->picked = true;
                closest_pick->point = closest_point;
                closest_pick->triangle = closest_id;
                auto pointcloud = std::dynamic_pointer_cast<Pointcloud>(closest_dr->mesh);
                if (pointcloud != nullptr)
                {
                    closest_pick->primitive = static_cast<size_t>(
                        std::floor((double)closest_id / (double)pointcloud->trianglesPerPoint()));
                }
            }
        }
    }
}

unsigned int PickSystem::priority() const
{
    return 1200;
}

const std::string PickSystem::name() const
{
    return "PickSystem";
}

} // namespace viewer
} // namespace rcube