#ifndef ARCBALLCONTROLLER_H
#define ARCBALLCONTROLLER_H

#include <memory>
#include "glm/glm.hpp"
#include "CameraController.h"
#include "Arcball.h"

namespace rcube {

/**
 * @brief The ArcballController class
 */
class ArcballController : public Controller {
public:
    ArcballController() {
        rotate_button_ = MouseButton::Right;
        pan_button_ = MouseButton::Middle;
    }
    float getSpeed() const {
        return arcball_.speed();
    }
    void setSpeed(float val) {
        arcball_.setSpeed(val);
    }
    void mouseDown(int x, int y, MouseButton b) override {
        if (b == rotate_button_) {
            if (!arcball_.active()) {
                arcball_.start(x, y);
            }
        }
        else if (b == pan_button_) {
            if (!panning_) {
                panning_ = true;
                pan_start_x_ = x;
                pan_start_y_ = y;
            }
        }
    }
    void mouseMove(int x, int y, MouseButton b) override {
        if (b == rotate_button_) {
            arcball_.rotate(x, y);
            glm::vec3 pos = cam_->position();
            glm::vec3 tgt = cam_->target();
            glm::vec3 up = cam_->up();
            glm::mat4 minv = glm::inverse(arcball_.matrix());
            cam_->lookAt(tgt + glm::vec3(minv * glm::vec4(pos - tgt, 0)), cam_->target(), glm::mat3(minv) * up);
            arcball_.reset(x, y);
        }
        else if (b == pan_button_) {
            if (panning_) {
                float dx = static_cast<float>(x - pan_start_x_) / arcball_.width();
                float dy = -static_cast<float>(y - pan_start_y_) / arcball_.height();
                cam_->track(dx, dy);
                pan_start_x_ = x;
                pan_start_y_ = y;
            }
        }
    }
    void mouseUp(int x, int y, MouseButton b) override {
        if (b == rotate_button_) {
            if (arcball_.active()) {
                arcball_.stop(x, y);
            }
        }
        else if (b == pan_button_) {
            panning_ = false;
        }
    }
    void scroll(float /* dx */, float dy) override {
        if (cam_->isOrthographicProjection()) {
            cam_->setOrthoGraphicSize(cam_->orthographicSize() + dy);
        }
        else {
            cam_->dolly(dy);
        }
    }
    virtual void resize(float viewport_width, float viewport_height) override {
        arcball_.resize(viewport_width, viewport_height);
        Controller::resize(viewport_width, viewport_height);
    }

private:
    Arcball arcball_;
    MouseButton rotate_button_, pan_button_;
    bool panning_;
    int pan_start_x_, pan_start_y_;
};

} // namespace rcube

#endif // ARCBALLCONTROLLER_H
