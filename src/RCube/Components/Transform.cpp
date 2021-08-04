#include "RCube/Components/Transform.h"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "imgui.h"

namespace rcube
{

Transform::Transform()
    : position_(0, 0, 0), scale_(1, 1, 1), orientation_(1, 0, 0, 0), local_transform_(glm::mat4(1)),
      world_transform_(glm::mat4(1)), parent_(nullptr), dirty_(true)
{
}

Transform *Transform::parent() const
{
    return parent_;
}

void Transform::setParent(Transform *p)
{
    parent_ = p;
    p->children_.push_back(this);
}

glm::vec3 Transform::worldPosition() const
{
    return glm::vec3(world_transform_[3][0], world_transform_[3][1], world_transform_[3][2]);
}

const glm::vec3 &Transform::position() const
{
    return position_;
}

const glm::quat &Transform::orientation() const
{
    return orientation_;
}

const glm::vec3 &Transform::scale() const
{
    return scale_;
}

void Transform::setPosition(const glm::vec3 &pos)
{
    position_ = pos;
    dirty_ = true;
}

void Transform::setOrientation(const glm::quat &ort)
{
    orientation_ = ort;
    dirty_ = true;
}

void Transform::setScale(const glm::vec3 &sc)
{
    scale_ = sc;
    dirty_ = true;
}

const glm::mat4 &Transform::localTransform()
{
    return local_transform_;
}

const glm::mat4 &Transform::worldTransform()
{
    return world_transform_;
}

const std::vector<Transform *> Transform::children() const
{
    return children_;
}

void Transform::translate(const glm::vec3 &tr)
{
    position_ += tr;
    dirty_ = true;
}

void Transform::rotate(const glm::quat &quaternion)
{
    orientation_ *= quaternion;
    dirty_ = true;
}

void Transform::lookAt(const glm::vec3 &position, const glm::vec3 &target, const glm::vec3 &up)
{
    setPosition(position);
    setOrientation(
        glm::normalize(glm::quatLookAt(glm::normalize(target - position), glm::normalize(up))));
    dirty_ = true;
}

void Transform::drawGUI()
{
    // TODO: think of a way to handle transform hierarchy
    if (ImGui::InputFloat3("Position", glm::value_ptr(position_), "%.2f"))
    {
        dirty_ = true;
    }

    glm::vec3 euler = glm::eulerAngles(orientation_);
    if (ImGui::SliderAngle("Orientation X", glm::value_ptr(euler)))
    {
        setOrientation(glm::quat(euler));
    }
    if (ImGui::SliderAngle("Orientation Y", glm::value_ptr(euler) + 2))
    {
        setOrientation(glm::quat(euler));
    }
    if (ImGui::SliderAngle("Orientation Z", glm::value_ptr(euler) + 1))
    {
        setOrientation(glm::quat(euler));
    }
    if (ImGui::InputFloat3("Scale", glm::value_ptr(scale_), "%.2f"))
    {
        dirty_ = true;
    }
}

void Transform::drawTransformWidget(const glm::mat4 &camera_world_to_view,
                                    const glm::mat4 &camera_view_to_projection,
                                    TransformOperation transformation,
                                    ImGuizmo::MODE transformation_space, glm::vec3 snap_translate,
                                    float snap_angle_degrees, float snap_scale)
{
    if (transformation == TransformOperation::None)
    {
        return;
    }
    float *snap = nullptr;
    if (transformation == TransformOperation::Translate)
    {
        snap = glm::value_ptr(snap_translate);
    }
    else if (transformation == TransformOperation::Rotate)
    {
        snap = &snap_angle_degrees;
    }
    else if (transformation == TransformOperation::Scale)
    {
        snap = &snap_scale;
    }
    glm::mat4 matrix = glm::mat4(localTransform());
    if (ImGuizmo::Manipulate(glm::value_ptr(camera_world_to_view),
                             glm::value_ptr(camera_view_to_projection),
                             static_cast<ImGuizmo::OPERATION>(transformation), transformation_space,
                             glm::value_ptr(matrix), nullptr, snap))
    {
        glm::vec3 translation, scale, euler_angles;
        ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(matrix), glm::value_ptr(translation),
                                              glm::value_ptr(euler_angles), glm::value_ptr(scale));
        if (transformation == TransformOperation::Rotate)
        {
            euler_angles = glm::radians(euler_angles);
            setOrientation(glm::quat(euler_angles));
        }
        else if (transformation == TransformOperation::Translate)
        {
            setPosition(translation);
        }
        else if (transformation == TransformOperation::Scale)
        {
            setScale(scale);
        }
    }
}

} // namespace rcube
