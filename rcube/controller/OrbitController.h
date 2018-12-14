#ifndef ORBITCONTROLLER_H
#define ORBITCONTROLLER_H

#include "glm/glm.hpp"
#include "PanZoomController.h"
#include "../components/Camera.h"

namespace rcube {

class OrbitController : public PanZoomController {
public:
    float min_horizontal_angle, max_horizontal_angle;
    float min_vertical_angle, max_vertical_angle;
    float orbit_speed = 1.0f;

    OrbitController();
    virtual void update(const CameraController::InputState &state) override;
private:
    bool panning_, orbiting_;
    int last_x_, last_y_;
};

} // namespace rcube

#endif // ORBITCONTROLLER_H
