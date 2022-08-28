#include <limits>
#include "glm/glm.hpp"
#include "glm/gtc/constants.hpp"
#include "RCube/Components/Camera.h"
#include "RCube/Components/Transform.h"

namespace rcube
{

class OrbitCameraController
{
  public:
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
    float pan_speed = 1.0f;    /// Speed of panning motion
    float zoom_speed = 0.5f;   /// Speed of zooming motion

    void drawGUI();

    void startRotation(double x, double y);

    void startRotation(const glm::dvec2 &pos);

    void stopRotation();

    void startPanning(double x, double y);

    void startPanning(const glm::dvec2 &pos);

    void stopPanning();

    void rotate(double x, double y);

    void pan(double x, double y);

    void zoom(double amount);

    void setCamera(Camera *cam, Transform *tr);

  private:
    bool orbiting_ = false;
    double last_ox_ = 0;
    double last_oy_ = 0;
    bool panning_ = false;
    double last_px_ = 0;
    double last_py_ = 0;
    double min_zoom_ = 0.1;
    Camera *camera_ = nullptr;
    Transform *transform_ = nullptr;
};
}