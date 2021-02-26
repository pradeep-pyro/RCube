#pragma once

#include "RCube/Core/Arch/Component.h"
#include "RCube/Window.h"
#include "glm/gtc/constants.hpp"

namespace rcube
{
namespace viewer
{

class CameraControllerSystem;

class CameraController : public Component<CameraController>
{
  public:
    bool active = true;
    InputState::Mouse pan = InputState::Mouse::Middle;
    InputState::Mouse rotate = InputState::Mouse::Right;
    float min_horizontal_angle =
        -std::numeric_limits<float>::infinity(); /// Minimum horizontal angle (radians) to constrain
                                                 /// the camera
                                                 /// (default: -inf)
    float max_horizontal_angle =
        std::numeric_limits<float>::infinity(); /// Maximum horizontal angle (radians) to constrain
                                                /// the camera (default: +inf)
    float min_vertical_angle =
        -glm::pi<float>(); /// Minimum vertical angle (radians) to constrain the camera (default:
                           /// -\pi)
    float max_vertical_angle =
        glm::pi<float>(); /// Maximum vertical angle (radians) to constrain the camera (default:
                          /// +\pi)
    float rotate_speed = 4.0f; /// Speed of orbiting motion
    float pan_speed = 4.0f;    /// Speed of panning motion
    float zoom_speed = 0.5f;   /// Speed of zooming motion

    void drawGUI();

  private:
    friend class CameraControllerSystem;
    float width_ = 1280;
    float height_ = 720;
    bool orbiting_ = false;
    int last_ox_ = 0;
    int last_oy_ = 0;
    bool panning_ = false;
    int last_px_ = 0;
    int last_py_ = 0;
};

} // namespace viewer
} // namespace rcube
