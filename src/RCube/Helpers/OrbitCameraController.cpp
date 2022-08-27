#include "RCube/Helpers/OrbitCameraController.h"
#include "imgui.h"

namespace rcube
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

void OrbitCameraController::drawGUI()
{
    ImGui::SliderFloat("Rotation speed", &rotate_speed, 1, 10);
    ImGui::SliderFloat("Pan speed", &pan_speed, 1, 10);
    ImGui::SliderFloat("Zoom speed", &zoom_speed, 0.5f, 10);
    ImGui::Separator();
    ImGui::SliderAngle("Min. horizonal angle", &min_horizontal_angle);
    ImGui::SliderAngle("Max. horizonal angle", &max_horizontal_angle);
    ImGui::SliderAngle("Min. vertical angle", &min_vertical_angle, -glm::pi<float>(),
                       glm::pi<float>());
    ImGui::SliderAngle("Max. vertical angle", &max_vertical_angle, -glm::pi<float>(),
                       glm::pi<float>());
}

void OrbitCameraController::startRotation(double x, double y)
{
    orbiting_ = true;
    panning_ = false;
    last_ox_ = x;
    last_oy_ = y;
}

void OrbitCameraController::startRotation(const glm::dvec2 &pos)
{
    startRotation(pos.x, pos.y);
}

void OrbitCameraController::stopRotation()
{
    orbiting_ = false;
    //last_ox_ = x;
    //last_oy_ = y;
}

void OrbitCameraController::startPanning(double x, double y)
{
    panning_ = true;
    orbiting_ = false;
    last_px_ = x;
    last_py_ = y;
}

void OrbitCameraController::startPanning(const glm::dvec2 &pos)
{
    startPanning(pos.x, pos.y);
}

void OrbitCameraController::stopPanning()
{
    panning_ = false;
    //last_px_ = x;
    //last_py_ = y;
}

void OrbitCameraController::rotate(double x, double y)
{
    if (camera_ == nullptr || transform_ == nullptr)
    {
        return;
    }
    if (orbiting_ && (last_ox_ != x) && (last_oy_ != y))
    {
        glm::vec3 pos = transform_->position() - camera_->target;
        float r, ha, va;
        cartesianToSpherical(pos, r, ha, va);
        const float dx = rotate_speed * static_cast<float>(last_ox_ - x) /
                         static_cast<float>(camera_->viewport_size[0]);
        const float dy = rotate_speed * static_cast<float>(last_oy_ - y) /
                         static_cast<float>(camera_->viewport_size[1]);
        va = glm::clamp(va + dy, min_vertical_angle, max_vertical_angle);
        ha = glm::clamp(ha + dx, min_horizontal_angle, max_horizontal_angle);
        pos = sphericalToCartesian(r, ha, va);
        transform_->lookAt(pos + camera_->target, camera_->target, YAXIS_POSITIVE);
        last_ox_ = x;
        last_oy_ = y;
    }
}

void OrbitCameraController::pan(double x, double y)
{
    if (camera_ == nullptr || transform_ == nullptr)
    {
        return;
    }
    if (panning_ && (last_px_ != x) && (last_py_ != y))
    {
        float dx = static_cast<float>(x - last_px_);
        float dy = static_cast<float>(y - last_py_);
        dx *= pan_speed;
        dy *= pan_speed;

        // Center to top of screen distance
        const glm::vec3 eye_to_target = camera_->target - transform_->worldPosition();
        float target_dist = glm::length(eye_to_target) * std::tan(camera_->fov * 0.5f);

        // Use height to normalize rather than width and height to avoid aspect ratio influences
        float pan_side_offset = (2 * dx * target_dist / camera_->viewport_size[1]);
        float pan_up_offset = (2 * dy * target_dist / camera_->viewport_size[1]);

        // Apply offset to target and eye
        const glm::vec3 up = glm::normalize(transform_->orientation() * YAXIS_POSITIVE);
        const glm::vec3 forward = glm::normalize(eye_to_target);
        const glm::vec3 side = glm::normalize(glm::cross(up, forward));
        transform_->translate(side * pan_side_offset + up * pan_up_offset);
        camera_->target += side * pan_side_offset + up * pan_up_offset;
        last_px_ = x;
        last_py_ = y;
    }
}

void OrbitCameraController::setCamera(Camera *cam, Transform *tr)
{
    camera_ = cam;
    transform_ = tr;
}

void OrbitCameraController::zoom(double amount)
{
    if (camera_ == nullptr || transform_ == nullptr)
    {
        return;
    }
    if (std::abs(amount) > 1e-6)
    {
        const glm::vec3 forward = glm::normalize(camera_->target - transform_->worldPosition());
        const glm::vec3 offset = forward * static_cast<float>(amount) * zoom_speed;
        glm::vec3 updated_world_pos = transform_->worldPosition() + offset;
        if (glm::dot(glm::normalize(camera_->target - updated_world_pos), forward) > 0)
        {
            // Dolly for perspective projection
            transform_->translate(offset);
            // Modify orthographic size for orthographic projection
            if (camera_->orthographic)
            {
                camera_->orthographic_size += static_cast<float>(amount) * zoom_speed;
            }
        }
    }
}

} // namespace rcube