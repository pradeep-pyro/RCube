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
}

EntityHandle World::createEntity() {
    return EntityHandle { entity_mgr_.createEntity(), this };
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
