#include "RCube/Components/Transform.h"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/matrix_decompose.hpp"
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
    scale_ = glm::clamp(scale_, glm::vec3(0), glm::vec3(1e10));
    dirty_ = true;
}

void Transform::scale(const glm::vec3 &sc)
{
    scale_ *= sc;
    scale_ = glm::clamp(scale_, glm::vec3(0), glm::vec3(1e10));
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

const std::vector<Transform *>& Transform::children() const
{
    return children_;
}

void Transform::translate(const glm::vec3 &tr)
{
    position_ += tr;
    dirty_ = true;
}

void Transform::rotate(const glm::quat &quaternion_model)
{
    orientation_ = orientation_ * quaternion_model;
    dirty_ = true;
}

glm::quat Transform::relativeRotation(const glm::quat &target, const glm::quat &current)
{
    return target * glm::inverse(current);
}

void Transform::rotateWorld(const glm::quat &quaternion_world)
{
    orientation_ = quaternion_world * orientation_;
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

const glm::mat4 Transform::parentTransform()
{
    return parent_ == nullptr ? glm::mat4(1) : parent_->world_transform_;
}

void Transform::translateWorld(const glm::vec3 &tr)
{
    glm::mat4 translation_matrix = glm::translate(tr);
    glm::mat4 updated_world_matrix = glm::inverse(this->parentTransform()) * translation_matrix *
                                     this->parentTransform() * this->localTransform();
    glm::vec3 uscal, utran, uskew;
    glm::vec4 upers;
    glm::quat uorie;
    glm::decompose(updated_world_matrix, uscal, uorie, utran, uskew, upers);
    this->position_ = utran;
    this->orientation_ = uorie;
    this->scale_ = uscal;
    dirty_ = true;
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
    glm::mat4 matrix = worldTransform();
    glm::mat4 delta = glm::identity<glm::mat4>();
    if (ImGuizmo::Manipulate(glm::value_ptr(camera_world_to_view),
                             glm::value_ptr(camera_view_to_projection),
                             static_cast<ImGuizmo::OPERATION>(transformation), transformation_space,
                             glm::value_ptr(matrix), glm::value_ptr(delta), snap))
    {
        glm::vec3 dtranslation, dscale, deuler_angles_deg;
        ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(delta), glm::value_ptr(dtranslation),
                                              glm::value_ptr(deuler_angles_deg),
                                              glm::value_ptr(dscale));
        if (transformation == TransformOperation::Rotate)
        {
            glm::vec3 euler_angles_rad = glm::radians(deuler_angles_deg);
            glm::quat rotation(euler_angles_rad);
            if (transformation_space == ImGuizmo::MODE::WORLD)
            {
                this->rotateWorld(rotation);
            }
            else
            {
                this->rotate(rotation);
            }
        }
        else if (transformation == TransformOperation::Translate)
        {
            if (transformation_space == ImGuizmo::MODE::WORLD)
            {
                this->translateWorld(dtranslation);
            }
            else
            {
                this->translate(dtranslation);
            }
        }
        else if (transformation == TransformOperation::Scale)
        {
            this->scale(dscale);
        }
    }
}

} // namespace rcube
