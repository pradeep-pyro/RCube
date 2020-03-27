#include "RCube/Components/Transform.h"

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
    setOrientation(glm::quatLookAt(glm::normalize(target - position), up));
    dirty_ = true;
}

} // namespace rcube
