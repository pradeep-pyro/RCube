#ifndef CAMERACONTROLLER_H
#define CAMERACONTROLLER_H

#include "../components/Camera.h"
#include "../components/Transform.h"

namespace rcube {

/**
 * The CameraController class is the base class from which different kinds of
 * controller classes can be built.
 * By default, the base class does not provide any functionality except holding the camera
 * entity which is to be controlled, and movement and rotation speeds.
 *
 * The class can be extended by overriding the update() method.
 */
class CameraController {
public:

    struct InputState {
        InputState() {
            clear();
        }
        int x, y;
        bool mouse_left, mouse_middle, mouse_right;
        bool alt, ctrl, shift;
        double scroll_x, scroll_y;
        void clear() {
            x = 0;
            y = 0;
            mouse_middle = false;
            mouse_right = false;
            alt = false;
            ctrl = false;
            shift = false;
            scroll_x = 0.0;
            scroll_y = 0.0;
        }
    };

    /**
     * Constructor
     */
    CameraController(int width, int height);
    /**
     * Virtual destructor
     */
    virtual ~CameraController() = default;

    /**
     * Set the size of the viewport when it is resized.
     */
    virtual void resize(float viewport_width, float viewport_height);

    /**
     * Set the camera to be controlled
     * @param EntityHandle with a Camera and Transform component
     */
    virtual void setEntity(EntityHandle entity);

    /**
     * Return a pointer to the camera component being controlled
     * @return Pointer to Camera
     */
    Camera * camera() const;

    /**
     * Updates the camera and its transformation based on the given
     * input state
     * @param state IO state
     */
    virtual void update(const InputState &state) = 0;

protected:
    Camera *camera_;
    Transform *transform_;
    float width_, height_;
};

} // namespace rcube

#endif // CAMERACONTROLLER_H
