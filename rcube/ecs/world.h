#ifndef WORLD_H
#define WORLD_H

#include <memory>
#include "entitymanager.h"
#include "componentmanager.h"
#include "system.h"

struct EntityHandle;

/**
 * World is the primary interface to the user and allows the user to create entities,
 * add/remove components to entities, and add/remove systems to process components.
 *
 * World acts glue between various component managers and systems and abstracts away the internal
 * representations of the ECS architecture.
 */
class World {
public:
    World() = default;

    virtual ~World() = default;

    /**
     * One time initialization routines. Calls initialize() in all available systems
     */
    virtual void initialize();

    /**
     * Cleanup routines. Calls cleanup() in all available systems
     */
    virtual void cleanup();

    /**
     * Adds a component of type ComponentType to the entity
     * An easier approach is to get an EntityHandle from create entity and
     * call entity_handle.add<ComponentType>();
     */
    template <typename ComponentType>
    void addComponent(Entity entity, const ComponentType &comp) {
        ComponentManager<ComponentType> *manager = getComponentManager<ComponentType>();
        manager->add(entity, comp);
        updateEntityToSystem<ComponentType>(entity, true);
    }

    /**
     * Remove the component of type ComponentType from the entity
     * An easier approach is to get an EntityHandle from create entity and
     * call entity_handle.remove<ComponentType>();
     */
    template <typename ComponentType>
    void removeComponent(Entity entity) {
        ComponentManager<ComponentType> *manager = getComponentManager<ComponentType>();
        manager->remove(entity);
        updateEntityToSystem<ComponentType>(entity, false);
    }

    /**
     * Gets the component of type ComponentType from the entity
     * An easier approach is to get an EntityHandle from create entity and
     * call entity_handle.get<ComponentType>();
     */
    template <typename ComponentType>
    ComponentType * getComponent(Entity entity) {
        ComponentManager<ComponentType> *manager = getComponentManager<ComponentType>();
        return manager->get(entity);
    }

    /**
     * Adds a system that will process certain components
     */
    void addSystem(std::unique_ptr<System> sys);

    /**
     * Creates an entity and returns an EntityHandle
     */
    EntityHandle createEntity();

    /**
     * Update the world (usually called in the game loop)
     */
    void update();

protected:

    void registerEntityToSystem(Entity e);
    void unregisterEntityToSystem(Entity e);

    template <typename ComponentType>
    void updateEntityToSystem(Entity ent, bool flag) {
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
 * This enables API like entity_handle.add(component) instead of world->addComponent(entity, component)
 */
struct EntityHandle {
    Entity entity;
    World *world;
    /**
     * Add a component to the entity
     * @param comp Component to add
     */
    template <typename T>
    void add(const T &comp) {
        world->addComponent(entity, comp);
    }
    /**
     * Remove the component of type T from the entity
     */
    template <typename T>
    void remove() {
        world->removeComponent<T>(entity);
    }
    /**
     * Get the component of type T from the entity
     * @return Pointer to the component which is actually stored in
     * the world's component manager
     */
    template <typename T>
    T * get() {
        return world->getComponent<T>(entity);
    }
};


#endif // WORLD_H
