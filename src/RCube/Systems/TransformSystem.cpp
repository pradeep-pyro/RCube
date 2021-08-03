#include "RCube/Systems/TransformSystem.h"
#include "RCube/Components/Transform.h"
#include "glm/gtx/string_cast.hpp"

namespace rcube
{

unsigned int TransformSystem::priority() const
{
    return 100;
}

void TransformSystem::updateHierarchy(Transform *comp, bool force)
{
    if (comp->dirty_ || force)
    {
        const glm::mat4 R = glm::toMat4(comp->orientation_);
        const glm::mat4 S = glm::scale(comp->scale());
        const glm::mat4 T = glm::translate(comp->position_);
        comp->local_transform_ = T * R * S;

        // Set world transformation := local transformation if there is no parent
        if (comp->parent() == nullptr)
        {
            comp->world_transform_ = comp->localTransform();
        }
        else
        {
            comp->world_transform_ = comp->parent()->worldTransform() * comp->localTransform();
        }
        comp->dirty_ = false;
        for (auto child : comp->children())
        {
            updateHierarchy(child, true);
        }
    }
    else
    {
        for (auto child : comp->children())
        {
            updateHierarchy(child, force);
        }
    }
}

TransformSystem::TransformSystem()
{
    ComponentMask transform_filter;
    transform_filter.set(Transform::family());
    addFilter(transform_filter);
}

void TransformSystem::initialize()
{
}

void TransformSystem::cleanup()
{
}
void TransformSystem::update(bool force)
{
    const auto &transformable_entities = registered_entities_[filters_[0]];
    for (const Entity &ent : transformable_entities)
    {
        Transform *comp = world_->getComponent<Transform>(ent);
        // Update hierarchy from root level nodes which do not have a parent
        if (comp->parent() == nullptr)
        {
            updateHierarchy(comp, force);
        }
    }
}

} // namespace rcube
