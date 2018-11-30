#include "CameraController.h"

namespace rcube {

CameraController::CameraController(int width, int height, float move_speed, float rotate_speed)
    : width_(width), height_(height), move_speed_(move_speed), rotate_speed_(rotate_speed) {
}

float CameraController::moveSpeed() const {
    return move_speed_;
}

void CameraController::setMoveSpeed(float val) {
    move_speed_ = val;
}

float CameraController::rotateSpeed() const {
    return rotate_speed_;
}

void CameraController::setRotateSpeed(float val) {
    rotate_speed_ = val;
}

void CameraController::resize(float viewport_width, float viewport_height) {
    width_ = viewport_width;
    height_ = viewport_height;
}

void CameraController::setEntity(EntityHandle entity) {
    camera_ = entity.get<Camera>();
    transform_ = entity.get<Transform>();
}

Camera * CameraController::camera() const {
    return camera_;
}

} // namespace rcube
