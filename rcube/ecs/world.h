#ifndef WORLD_H
#define WORLD_H

#include <iostream>
#include <memory>
#include "entitymanager.h"
#include "componentmanager.h"
#include "system.h"

struct EntityHandle;

class World {
public:
    World() = default;

    virtual ~World() = default;

    virtual void initialize();

    virtual void cleanup();

    template <typename ComponentType>
    void addComponent(Entity entity, const ComponentType &comp) {
        ComponentManager<ComponentType> *manager = getComponentManager<ComponentType>();
        manager->add(entity, comp);
        updateEntityToSystem<ComponentType>(entity, true);
    }

    template <typename ComponentType>
    void removeComponent(Entity entity) {
        ComponentManager<ComponentType> *manager = getComponentManager<ComponentType>();
        manager->remove(entity);
        updateEntityToSystem<ComponentType>(entity, false);
    }

    template <typename ComponentType>
    ComponentType * getComponent(Entity entity) {
        ComponentManager<ComponentType> *manager = getComponentManager<ComponentType>();
        return manager->get(entity);
    }

    void addSystem(std::unique_ptr<System> sys);

    EntityHandle createEntity();

    void update();

protected:

    void registerEntityToSystem(Entity e);
    void unregisterEntityToSystem(Entity e);

    template <typename ComponentType>
    void updateEntityToSystem(Entity ent, bool flag) {
        /*
        for (const auto &sys : systems_) {
            ComponentMask sys_mask = sys->signature();
            ComponentMask old_entity_mask = entity_masks_[ent];
            entity_masks_[ent].set(ComponentType::family(), flag);
            ComponentMask new_entity_mask = entity_masks_[ent];
            if (new_entity_mask.match(sys_mask) && !old_entity_mask.match(sys_mask)) {
                sys->registerEntity(ent);
            }
            else if (!new_entity_mask.match(sys_mask) && old_entity_mask.match(sys_mask)) {
                sys->unregisterEntity(ent);
            }
        }
        */
        ComponentMask old_entity_mask = entity_masks_[ent];
        entity_masks_[ent].set(ComponentType::family(), flag);
        ComponentMask new_entity_mask = entity_masks_[ent];

        for (auto &sys : systems_) {
            int i = 0;
            for (ComponentMask sys_mask : sys->filters()) {
                bool new_match = new_entity_mask.match(sys_mask);
                bool old_match = old_entity_mask.match(sys_mask);
                if (new_match && !old_match) {
                    sys->registerEntity(ent, sys_mask);
                }
                else if (!new_match && old_match) {
                    sys->unregisterEntity(ent, sys_mask);
                }
                i++;
            }

        }

    }
    template <typename ComponentType>
    ComponentManager<ComponentType> * getComponentManager() {
        auto it = component_mgrs_.find(ComponentType::family());
        if (it == component_mgrs_.end()) {
            component_mgrs_[ComponentType::family()] = std::make_unique<ComponentManager<ComponentType>>();
        }
        const auto &mgr = component_mgrs_[ComponentType::family()];
        return static_cast<ComponentManager<ComponentType> *>(mgr.get());
    }

    std::vector<std::unique_ptr<System>> systems_;
    std::map<int, std::unique_ptr<BaseComponentManager>> component_mgrs_;
    EntityManager entity_mgr_;
    std::map<Entity, ComponentMask> entity_masks_;
};

/**
 * Wraps Entity and pointer to World so that components can be added easily
 * i.e., entity_handle.add(component) instead of world->addComponent(entity, component)
 */
struct EntityHandle {
    Entity entity;
    World *world;
    template <typename T>
    void add(const T &comp) {
        world->addComponent(entity, comp);
    }
    template <typename T>
    void remove() {
        world->removeComponent<T>(entity);
    }
    template <typename T>
    T * get() {
        return world->getComponent<T>(entity);
    }
};


#endif // WORLD_H
