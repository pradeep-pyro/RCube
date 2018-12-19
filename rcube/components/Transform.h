#ifndef TRANSFORM_H
#define TRANSFORM_H

#include <vector>
#include "glm/glm.hpp"
#include "glm/gtx/quaternion.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/transform.hpp"
#include "../ecs/component.h"
#include "../ecs/componentmanager.h"
#include "../ecs/system.h"
#include "../ecs/world.h"

namespace rcube {

class TransformSystem;

/**
 * Transform represents the local position, orientation and scale of objects in the world
 *
 * It can be used to represent transform hierarchies (scene graphs) by assigning a
 * parent Transforms appropriately. By default, the parent is a nullptr.
 */
class Transform : public Component<Transform> {
public:
    Transform();

    /**
     * Returns the parent
     * @return Pointer to parent or nullptr if there is none
     */
    Transform * parent() const;

    /**
     * Sets the parent Transform to form a transform hierarchy
     * @param p Pointer to parent Transform (ownership not assumed)
     */
    void setParent(Transform *p);

    /**
     * Returns the position in world coordinates after resolving the
     * transform hierarchy
     * @return 3D world position
     */
    glm::vec3 worldPosition() const;

    /**
     * Returns the position in local coordinates with respect to the
     * parent Transform
     * @return 3D local position
     */
    const glm::vec3 & position() const;

    /**
     * Returns the position in local coordinates with respect to the
     * parent Transform
     * @return 3D local position
     */
    const glm::quat & orientation() const;

    /**
     * Returns the scale of the object in local coordinates with respect
     * to the parent Transform
     * @return 3D scale of the object
     */
    const glm::vec3 & scale() const;

    /**
     * Sets the position of the object in local coordinates with respect
     * to the parent Transform
     * @param pos Local position
     */
    void setPosition(const glm::vec3 &pos);

    /**
     * Sets the orientation of the object in local coordinates with respect
     * to the parent Transform
     * @param ort Orientation (unit quaternion)
     */
    void setOrientation(const glm::quat &ort);

    /**
     * Sets the scale of the object in local coordinates with respect
     * to the parent Transform
     * @param sc Scale (3D vector)
     */
    void setScale(const glm::vec3 &sc);

    /**
     * Returns the local transformation matrix
     * @return 4x4 transformation matrix combing rotation and translation
     */
    const glm::mat4 & localTransform();

    /**
     * Sets the local transformation matrix
     * @param matrix 4x4 transformation matrix combing rotation and translation
     */
    void setLocalTransform(const glm::mat4 &matrix);

    /**
     * Returns the global transformation matrix in world space
     * @return 4x4 transformation matrix combing rotation and translation
     */
    const glm::mat4 & worldTransform();

    /**
     * Sets the global transformation matrix in world space
     * @param matrix 4x4 transformation matrix combing rotation and translation
     */
    void setWorldTransform(const glm::mat4 &matrix);

    /**
     * Returns the children of the current Transform
     * @return list of children
     */
    const std::vector<Transform *> children() const;

    /**
     * Translate the object by adding the given vector to
     * the current position
     * @param tr 3D vector
     */
    void translate(const glm::vec3 &tr);

    /**
     * Rotate the object by multiplying the given quaternion with
     * current orientation
     * @param quaternion Unit quaternion
     */
    void rotate(const glm::quat &quaternion);
private:
    friend class TransformSystem;
    glm::vec3 position_, scale_;
    glm::quat orientation_;
    glm::mat4 local_transform_, world_transform_;
    Transform *parent_;
    std::vector<Transform *> children_;
    bool dirty_ = true;
};

} // namespace rcube

#endif // TRANSFORM_H
