#ifndef PANZOOMCONTROLLER_H
#define PANZOOMCONTROLLER_H

#include "RCube/Controller/CameraController.h"
namespace rcube {

/**
 * PanZoomController is used to allow the camera to be panned and zoomed
 */
class PanZoomController : public CameraController {
public:
    float pan_speed = 1.0f;   /// Panning speed
    float zoom_speed = 0.1f;  /// Zooming speed

    PanZoomController();
    void startPanning(int x, int y);
    void stopPanning(int x, int y);
    virtual void pan(int x, int y);
    virtual void zoom(float amount);
private:
    bool panning_ = false;
    int last_px_ = 0;
    int last_py_ = 0;
};

} // namespace rcube

#endif // PANZOOMCONTROLLER_H
