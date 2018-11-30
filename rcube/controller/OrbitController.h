#ifndef ORBITCONTROLLER_H
#define ORBITCONTROLLER_H

#include "glm/glm.hpp"
#include "CameraController.h"
#include "../components/Camera.h"

namespace rcube {

class OrbitController : public CameraController {
public:
    float min_horizontal_angle_, max_horizontal_angle_;
    float min_vertical_angle_, max_vertical_angle_;

    OrbitController();
    virtual void update(const CameraController::InputState &state) override;
private:
    bool panning_, orbiting_;
    int last_x_, last_y_;
};

} // namespace rcube

#endif // ORBITCONTROLLER_H
