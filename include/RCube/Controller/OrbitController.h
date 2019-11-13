#ifndef ORBITCONTROLLER_H
#define ORBITCONTROLLER_H

#include "glm/glm.hpp"
#include "RCube/Controller/PanZoomController.h"
#include "RCube/Components/Camera.h"

namespace rcube {

/**
 * OrbitController allows the camera to pan, zoom and orbit around the object
 * This class simply extends PanZoomController by adding orbiting functionality
 */
class OrbitController : public PanZoomController {
public:
    float min_horizontal_angle;  /// Minimum horizontal angle (radians) to constrain the camera (default: -inf)
    float max_horizontal_angle;  /// Maximum horizontal angle (radians) to constrain the camera (default: +inf)
    float min_vertical_angle;    /// Minimum vertical angle (radians) to constrain the camera (default: -\pi)
    float max_vertical_angle;    /// Maximum vertical angle (radians) to constrain the camera (default: +\pi)
    float orbit_speed = 1.0f;    /// Speed of orbiting motion

    OrbitController();
    void startOrbiting(int x, int y);
    void stopOrbiting(int x, int y);
    void orbit(int x, int y);
private:
    bool orbiting_;
    int last_ox_, last_oy_;
};

} // namespace rcube

#endif // ORBITCONTROLLER_H
