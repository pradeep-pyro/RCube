#include "RCube/Core/Arch/World.h"
#include <iostream>

namespace rcube
{

void World::initialize()
{
    for (const auto &sys : systems_)
    {
        sys->initialize();
    }
}

void World::cleanup()
{
    for (const auto &sys : systems_)
    {
        sys->cleanup();
    }
    systems_.clear();
    component_mgrs_.clear();
}

EntityHandle World::createEntity()
{
    return EntityHandle{entity_mgr_.createEntity(), this};
}

bool World::hasEntity(EntityHandle ent) const
{
    return entity_mgr_.hasEntity(ent.entity);
}

void World::removeEntity(EntityHandle ent)
{
    if (ent.world != this)
    {
        std::cerr << "Given EntityHandle was not generated from this World" << std::endl;
        return;
    }
    for (auto &mgr_ : component_mgrs_)
    {
        mgr_.second->remove(ent.entity);
        updateEntityToSystem(ent.entity, mgr_.first, false);
    }
    entity_mgr_.removeEntity(ent.entity);
}

EntityHandleIterator<std::unordered_set<Entity>> World::entities()
{
    return EntityHandleIterator<decltype(entity_mgr_.entities)>(this, entity_mgr_.entities);
}

size_t World::numEntities() const
{
    return entity_mgr_.count();
}

void World::update(float delta_time)
{
    for (const auto &sys : systems_)
    {
        sys->preUpdate();
    }
    for (const auto &sys : systems_)
    {
        sys->update(false);
    }
    for (const auto &sys : systems_)
    {
        sys->postUpdate();
    }
}

void World::addSystem(std::unique_ptr<System> sys)
{
    sys->registerWorld(this);
    systems_.push_back(std::move(sys));
    std::sort(systems_.begin(), systems_.end(),
              [](const std::unique_ptr<System> &sys1, const std::unique_ptr<System> &sys2) {
                  return sys1->priority() < sys2->priority();
              });
}

void World::updateEntityToSystem(Entity ent, int component_family, bool flag)
{
    ComponentMask old_entity_mask = entity_masks_[ent];
    entity_masks_[ent].set(component_family, flag);

    ComponentMask new_entity_mask = entity_masks_[ent];

    for (auto &sys : systems_)
    {
        for (ComponentMask sys_mask : sys->filters())
        {
            bool new_match = new_entity_mask.match(sys_mask);
            bool old_match = old_entity_mask.match(sys_mask);
            if (new_match && !old_match)
            {
                sys->registerEntity(ent, sys_mask);
            }
            else if (!new_match && old_match)
            {
                sys->unregisterEntity(ent, sys_mask);
            }
        }
    }
}

} // namespace rcube
