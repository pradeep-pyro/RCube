#ifndef PANZOOMCONTROLLER_H
#define PANZOOMCONTROLLER_H

#include "CameraController.h"

namespace rcube {

class PanZoomController : public CameraController {
public:
    float pan_speed = 1.0f;
    float zoom_speed = 1.0f;

    PanZoomController();
    virtual void update(const CameraController::InputState &state) override;
private:
    bool panning_;
    int last_x_, last_y_;
};

} // namespace rcube

#endif // PANZOOMCONTROLLER_H
