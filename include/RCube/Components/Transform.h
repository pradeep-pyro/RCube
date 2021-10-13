#pragma once

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/quaternion.hpp"
#include "glm/gtx/transform.hpp"
#include <vector>

#include "RCube/Core/Arch/Component.h"
#include "RCube/Core/Arch/ComponentManager.h"
#include "RCube/Core/Arch/System.h"
#include "RCube/Core/Arch/World.h"
#include "RCube/ImGuizmo.h"

namespace rcube
{

enum class TransformOperation
{
    None,
    Translate = ImGuizmo::TRANSLATE,
    Rotate = ImGuizmo::ROTATE,
    Scale = ImGuizmo::SCALE,
};

class TransformSystem;

/**
 * Transform represents the local position, orientation and scale of objects in the world
 *
 * It can be used to represent transform hierarchies (scene graphs) by assigning a
 * parent Transforms appropriately. By default, the parent is a nullptr.
 */
class Transform : public Component<Transform>
{
  public:
    Transform();

    static glm::quat relativeRotation(const glm::quat &target, const glm::quat &current);

    /**
     * Returns the parent
     * @return Pointer to parent or nullptr if there is none
     */
    Transform *parent() const;

    /**
     * Sets the parent Transform to form a transform hierarchy
     * @param p Pointer to parent Transform (ownership not assumed)
     */
    void setParent(Transform *p);

    /**
     * Returns the position in world coordinates.
     * NOTE: the transform hierarchy is only resolved during the next iteration of the
     * game loop. Accessing worldPosition() immediately after updating the transform
     * will give results from the previous computation.
     * transform hierarchy
     * @return 3D world position
     */
    glm::vec3 worldPosition() const;

    /**
     * Returns the position in local coordinates with respect to the
     * parent Transform
     * @return 3D local position
     */
    const glm::vec3 &position() const;

    /**
     * Returns the position in local coordinates with respect to the
     * parent Transform
     * @return 3D local position
     */
    const glm::quat &orientation() const;

    /**
     * Returns the scale of the object in local coordinates with respect
     * to the parent Transform
     * @return 3D scale of the object
     */
    const glm::vec3 &scale() const;

    /**
     * Scale the object in local coordinates with respect
     * to the parent Transform
     * @param[in] 3D scale of the object
     */
    void scale(const glm::vec3 &sc);

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
     * @return 4x4 transformation matrix combining scale, rotation and translation
     */
    const glm::mat4 &localTransform();

    /**
     * Sets the local transformation matrix and extracts the position, orientation
     * and scale from it.
     *
     * @param mat 4x4 transformation matrix combining scale, rotation and translation
     */
    void setLocalTransform(const glm::mat4 &mat);

    /**
     * Returns the global transformation matrix in world space
     * @return 4x4 transformation matrix combining rotation and translation
     */
    const glm::mat4 &worldTransform();

    /**
     * Returns the global transformation matrix in world space of the parent
     * transform, or an identity matrix if there's no parent
     * @return 4x4 transformation matrix combining rotation and translation
     */
    const glm::mat4 parentTransform();

    /**
     * Returns the children of the current Transform
     * @return list of children
     */
    const std::vector<Transform *> &children() const;

    /**
     * Translate the object by adding the given vector to
     * the current position
     * @param tr 3D vector
     */
    void translate(const glm::vec3 &tr);

    /**
     * Rotate the object in local space by right multiplying the given quaternion with
     * current orientation
     * @param quaternion Unit quaternion in model space
     */
    void rotate(const glm::quat &quaternion_model);

    /**
     * Rotate the object in world space by left multiplying the given quaternion with
     * current orientation
     * @param quaternion Unit quaternion in world space
     */
    void rotateWorld(const glm::quat &quaternion_world);

    /**
     * Transform such that the object is in given the lookAt configuration
     * @param position Position of the transform
     * @param target Target to look at
     * @param up Up orientation of the object
     */
    void lookAt(const glm::vec3 &position, const glm::vec3 &target, const glm::vec3 &up);

    void drawGUI();

    void translateWorld(const glm::vec3 &tr);

    void drawTransformWidget(const glm::mat4 &camera_world_to_view,
                             const glm::mat4 &camera_view_to_projection,
                             TransformOperation transformation, ImGuizmo::MODE transformation_space,
                             glm::vec3 snap_translate, float snap_angle_degrees, float snap_scale);

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
