#ifndef PANZOOMCONTROLLER_H
#define PANZOOMCONTROLLER_H

#include "CameraController.h"

namespace rcube {

/**
 * PanZoomController is used to allow the camera to be panned and zoomed
 */
class PanZoomController : public CameraController {
public:
    float pan_speed = 1.0f;   /// Panning speed
    float zoom_speed = 0.1f;  /// Zooming speed

    PanZoomController();
    virtual void update(const CameraController::InputState &state) override;
private:
    bool panning_;
    int last_x_, last_y_;
};

} // namespace rcube

#endif // PANZOOMCONTROLLER_H
