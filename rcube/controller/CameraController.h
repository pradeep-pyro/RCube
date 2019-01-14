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
 * The class can be extended by adding methods to modify the stored camera or its transform.
 */
class CameraController {
public:
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

protected:
    Camera *camera_;
    Transform *transform_;
    float width_, height_;
};

} // namespace rcube

#endif // CAMERACONTROLLER_H
