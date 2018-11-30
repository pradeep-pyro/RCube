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

    const glm::vec3 & scale() const;

    void setPosition(const glm::vec3 &pos);

    void setOrientation(const glm::quat &ort);

    void setScale(const glm::vec3 &sc);

    const glm::mat4 & localTransform();

    void setLocalTransform(const glm::mat4 &matrix);

    const glm::mat4 & worldTransform();

    void setWorldTransform(const glm::mat4 &matrix);

    bool dirty() const;

    void setDirty(bool flag);

    const std::vector<Transform *> children() const;

    void translate(const glm::vec3 &tr);

    void rotate(const glm::quat &quaternion);
private:
    glm::vec3 position_, scale_;
    glm::quat orientation_;
    glm::mat4 local_transform_, world_transform_;
    Transform *parent_;
    std::vector<Transform *> children_;
    bool dirty_;
};

} // namespace rcube

#endif // TRANSFORM_H
