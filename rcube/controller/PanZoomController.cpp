#include "PanZoomController.h"

namespace rcube {

PanZoomController::PanZoomController() : CameraController(1280, 720), panning_(false) {
}

void PanZoomController::update(const CameraController::InputState &state) {
    if (panning_ != state.mouse_middle) {
        panning_ = state.mouse_middle;
        if (panning_) {
            last_x_ = state.x;
            last_y_ = state.y;
        }
    }
    if (panning_ && last_x_ != state.x && last_y_ != state.y) {
        float dx = static_cast<float>(state.x - last_x_) / width_;
        float dy = -static_cast<float>(state.y - last_y_) / height_;
        dx *= pan_speed;
        dy *= pan_speed;
        glm::vec3 side = glm::cross(camera_->up, camera_->target - transform_->worldPosition());
        side = glm::normalize(side);
        transform_->translate(side * dx - camera_->up * dy);
        camera_->target += side * dx - camera_->up * dy;
        last_x_ = state.x;
        last_y_ = state.y;
    }

    // Zoom (actually dolly)
    if (std::abs(state.scroll_y) > 1e-6) {
        glm::vec3 forward = glm::normalize(camera_->target - transform_->worldPosition());
        transform_->translate(forward * float(state.scroll_y) * zoom_speed);
    }
}

} // namespace rcube
