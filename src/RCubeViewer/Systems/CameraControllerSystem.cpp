#include "RCubeViewer/Systems/CameraControllerSystem.h"

namespace rcube
{
namespace viewer
{

void cartesianToSpherical(const glm::vec3 &v, float &r, float &azimuth, float &polar,
                          float polar_eps = 1e-3f)
{
    r = glm::length(v);
    if (r < 1e-6f)
    {
        azimuth = 0.f;
        polar = 0.f;
    }
    else
    {
        azimuth = std::atan2(v.x, v.z);
        polar = glm::acos(glm::clamp(v.y / r, -1.f, 1.f));
    }
    polar = glm::clamp(polar, polar_eps, glm::pi<float>() - polar_eps);
}

glm::vec3 sphericalToCartesian(float r, float azimuth, float polar)
{
    float x = r * glm::sin(polar) * glm::sin(azimuth);
    float y = r * glm::cos(polar);
    float z = r * glm::sin(polar) * glm::cos(azimuth);
    return glm::vec3(x, y, z);
}

CameraControllerSystem::CameraControllerSystem() : System()
{
    addFilter({Camera::family(), Transform::family(), CameraController::family()});
}
void CameraControllerSystem::update(bool /*force*/)
{
    const int x = static_cast<int>(InputState::instance().mousePos()[0]);
    const int y = static_cast<int>(InputState::instance().mousePos()[1]);
    const glm::dvec2 scroll = InputState::instance().scrollAmount();
    for (const Entity &ent : getFilteredEntities(
             {Camera::family(), Transform::family(), CameraController::family()}))
    {
        CameraController *opz = world_->getComponent<CameraController>(ent);
        if (!opz->active)
        {
            continue;
        }
        Camera *cam = world_->getComponent<Camera>(ent);
        Transform *tr = world_->getComponent<Transform>(ent);
        opz->width_ = static_cast<float>(cam->viewport_size.x);
        opz->height_ = static_cast<float>(cam->viewport_size.y);

        /////////////////////////////////////////////////////////
        // Orbit
        /////////////////////////////////////////////////////////
        if (InputState::instance().mouseState(opz->rotate) == InputState::ButtonState::JustDown)
        {
            opz->orbiting_ = true;
            opz->last_ox_ = x;
            opz->last_oy_ = y;
        }
        else if (InputState::instance().mouseState(opz->rotate) ==
                 InputState::ButtonState::JustReleased)
        {
            opz->orbiting_ = false;
            opz->last_ox_ = x;
            opz->last_oy_ = y;
        }
        if (opz->orbiting_ && (opz->last_ox_ != x) && (opz->last_oy_ != y))
        {
            glm::vec3 pos = tr->position() - cam->target;
            float r, ha, va;
            cartesianToSpherical(pos, r, ha, va);
            const float dx = opz->rotate_speed * static_cast<float>(opz->last_ox_ - x) /
                             static_cast<float>(cam->viewport_size[0]);
            const float dy = opz->rotate_speed * static_cast<float>(opz->last_oy_ - y) /
                             static_cast<float>(cam->viewport_size[1]);
            va = glm::clamp(va + dy, opz->min_vertical_angle, opz->max_vertical_angle);
            ha = glm::clamp(ha + dx, opz->min_horizontal_angle, opz->max_horizontal_angle);
            pos = sphericalToCartesian(r, ha, va);
            tr->lookAt(pos + cam->target, cam->target, YAXIS_POSITIVE);
            opz->last_ox_ = x;
            opz->last_oy_ = y;
        }
        /////////////////////////////////////////////////////////
        // Pan
        /////////////////////////////////////////////////////////
        if (InputState::instance().mouseState(opz->pan) == InputState::ButtonState::JustDown)
        {
            opz->panning_ = true;
            opz->last_px_ = x;
            opz->last_py_ = y;
        }
        else if (InputState::instance().mouseState(opz->pan) ==
                 InputState::ButtonState::JustReleased)
        {
            opz->panning_ = false;
            opz->last_px_ = x;
            opz->last_py_ = y;
        }
        if (opz->panning_ && (opz->last_px_ != x) && (opz->last_py_ != y))
        {
            float dx = static_cast<float>(x - opz->last_px_) / opz->width_;
            float dy = -static_cast<float>(y - opz->last_py_) / opz->height_;
            dx *= opz->pan_speed;
            dy *= opz->pan_speed;
            const glm::vec3 up = glm::normalize(tr->orientation() * YAXIS_POSITIVE);
            const glm::vec3 forward = glm::normalize(cam->target - tr->worldPosition());
            const glm::vec3 side = glm::normalize(glm::cross(up, forward));
            tr->translate(side * dx - up * dy);
            cam->target += side * dx - up * dy;
            opz->last_px_ = x;
            opz->last_py_ = y;
        }
        /////////////////////////////////////////////////////////
        // Zoom
        /////////////////////////////////////////////////////////
        if (std::abs(scroll.y) > 1e-6)
        {
            const glm::vec3 forward = glm::normalize(cam->target - tr->worldPosition());
            // Dolly for perspective projection
            tr->translate(forward * static_cast<float>(scroll.y) * opz->zoom_speed);
            // Modify orthographic size for orthographic projection
            if (cam->orthographic)
            {
                cam->orthographic_size += static_cast<float>(scroll.y) * opz->zoom_speed;
            }
        }
    }
}

const std::string CameraControllerSystem::name() const
{
    return "CameraControllerSystem";
}

unsigned int CameraControllerSystem::priority() const
{
    return 1000;
}

} // namespace viewer
} // namespace rcube