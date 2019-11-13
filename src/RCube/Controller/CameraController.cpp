#include "RCube/Controller/CameraController.h"

namespace rcube
{

CameraController::CameraController(int width, int height) : width_(width), height_(height)
{
}

void CameraController::resize(float viewport_width, float viewport_height)
{
    width_ = viewport_width;
    height_ = viewport_height;
}

void CameraController::setEntity(EntityHandle entity)
{
    camera_ = entity.get<Camera>();
    transform_ = entity.get<Transform>();
}

Camera *CameraController::camera() const
{
    return camera_;
}

} // namespace rcube
