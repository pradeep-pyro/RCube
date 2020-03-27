#include "RCube/Controller/PanZoomController.h"
#include "glm/gtx/string_cast.hpp"

namespace rcube
{

PanZoomController::PanZoomController() : CameraController(1280, 720), panning_(false)
{
}

void PanZoomController::startPanning(int x, int y)
{
    panning_ = true;
    last_px_ = x;
    last_py_ = y;
}

void PanZoomController::stopPanning(int x, int y)
{
    panning_ = false;
    last_px_ = x;
    last_py_ = y;
}

void PanZoomController::pan(int x, int y)
{
    if (panning_ && (last_px_ != x) && (last_py_ != y))
    {
        float dx = static_cast<float>(x - last_px_) / width_;
        float dy = -static_cast<float>(y - last_py_) / height_;
        dx *= pan_speed;
        dy *= pan_speed;
        const glm::vec3 up = glm::normalize(transform_->orientation() * YAXIS_POSITIVE);
        const glm::vec3 forward = glm::normalize(camera_->target - transform_->worldPosition());
        const glm::vec3 side = glm::normalize(glm::cross(up, forward));
        transform_->translate(side * dx - up * dy);
        camera_->target += side * dx - up * dy;
        last_px_ = x;
        last_py_ = y;
    }
}

void PanZoomController::zoom(float amount)
{
    // Zoom
    if (std::abs(amount) > 1e-6)
    {
        const glm::vec3 forward = glm::normalize(camera_->target - transform_->worldPosition());
        // Dolly for perspective projection
        transform_->translate(forward * float(amount) * zoom_speed);
        translate(forward * amount * zoom_speed);
        if (camera_->orthographic)
        {
            camera_->orthographic_size += amount * zoom_speed;
        }
    }
}

} // namespace rcube
