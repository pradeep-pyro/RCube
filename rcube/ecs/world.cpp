#include <iostream>
#include "world.h"

void World::initialize() {
    for (const auto &sys : systems_) {
        sys->initialize();
    }
}

void World::cleanup() {
    for (const auto &sys : systems_) {
        sys->cleanup();
    }
    systems_.clear();
    component_mgrs_.clear();
}

EntityHandle World::createEntity() {
    return EntityHandle { entity_mgr_.createEntity(), this };
}

bool World::hasEntity(EntityHandle ent) const {
    return entity_mgr_.hasEntity(ent.entity);
}

void World::removeEntity(EntityHandle ent) {
    if (ent.world != this) {
        std::cerr << "Given EntityHandle was not generated from this World" << std::endl;
        return;
    }
    for (auto &mgr_ : component_mgrs_) {
        mgr_.second->remove(ent.entity);
    }
    for (auto &sys_ : systems_) {
        sys_->unregisterEntity(ent.entity);
    }
    entity_mgr_.removeEntity(ent.entity);
}

EntityHandleIterator<std::unordered_set<Entity>> World::entities() {
    return EntityHandleIterator<decltype(entity_mgr_.entities)>(this, entity_mgr_.entities);
}

void World::update() {
    for (const auto &sys : systems_) {
        sys->update(false);
    }
}

void World::addSystem(std::unique_ptr<System> sys) {
    sys->registerWorld(this);
    systems_.push_back(std::move(sys));
}
