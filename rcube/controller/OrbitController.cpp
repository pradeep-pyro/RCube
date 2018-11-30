#include "OrbitController.h"
#include <limits>
#include "glm/gtc/constants.hpp"
#include "glm/gtx/string_cast.hpp"

namespace rcube {

void cartesianToSpherical(const glm::vec3 &v, float &r, float &azimuth, float &polar,
                          float polar_eps=1e-3f) {
    r = glm::length(v);
    if (r < 1e-6f) {
        azimuth = 0.f;
        polar = 0.f;
    }
    else {
        azimuth = std::atan2(v.x, v.z);
        polar = glm::acos(glm::clamp(v.y / r, -1.f, 1.f));
    }
    polar = glm::clamp(polar, polar_eps, glm::pi<float>() - polar_eps);
}

glm::vec3 sphericalToCartesian(float r, float azimuth, float polar) {
    float x = r * glm::sin(polar) * glm::sin(azimuth);
    float y = r * glm::cos(polar);
    float z = r * glm::sin(polar) * glm::cos(azimuth);
    return glm::vec3(x, y, z);
}

OrbitController::OrbitController() : CameraController(1280, 720), panning_(false), orbiting_(false),
        min_horizontal_angle_(-std::numeric_limits<float>::infinity()), max_horizontal_angle_(std::numeric_limits<double>::infinity()),
        min_vertical_angle_(0.1f), max_vertical_angle_(glm::pi<float>() - 0.1f) {
}

void OrbitController::update(const CameraController::InputState &state) {
    if (orbiting_ != state.mouse_right) {
        orbiting_ = state.mouse_right;
        if (orbiting_) {
            last_x_ = state.x;
            last_y_ = state.y;
        }
    }

    if (panning_ != state.mouse_middle) {
        panning_ = state.mouse_middle;
        if (panning_) {
            last_x_ = state.x;
            last_y_ = state.y;
        }
    }

    if (orbiting_ && last_x_ != state.x && last_y_ != state.y) {
        glm::vec3 pos = transform_->position();
        float r, ha, va;
        cartesianToSpherical(pos, r, ha, va);
        float dx = static_cast<float>(last_x_ - state.x) / static_cast<float>(width_);
        float dy = static_cast<float>(last_y_ - state.y) / static_cast<float>(height_);
        dx *= move_speed_;
        dy *= rotate_speed_;
        va = glm::clamp(va + dy, min_vertical_angle_, max_vertical_angle_);
        ha = glm::clamp(ha + dx, min_horizontal_angle_, max_horizontal_angle_);
        pos = sphericalToCartesian(r, ha, va);
        transform_->setPosition(pos);
        last_x_ = state.x;
        last_y_ = state.y;
    }

    if (panning_ && last_x_ != state.x && last_y_ != state.y) {
        float dx = static_cast<float>(state.x - last_x_) / width_;
        float dy = -static_cast<float>(state.y - last_y_) / height_;
        dx *= move_speed_;
        dy *= move_speed_;
        transform_->translate(glm::vec3(dx, dy, 0));
        last_x_ = state.x;
        last_y_ = state.y;
    }
}

} // namespace rcube
