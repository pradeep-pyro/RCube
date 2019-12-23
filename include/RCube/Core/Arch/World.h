#ifndef WORLD_H
#define WORLD_H

#include <memory>

#include "RCube/Core/Arch/ComponentManager.h"
#include "RCube/Core/Arch/EntityManager.h"
#include "RCube/Core/Arch/System.h"

namespace rcube
{

struct EntityHandle;

template <typename Container> class EntityHandleIterator;

/**
 * World is the primary interface and allows the user to create entities,
 * add/remove components to entities, and add/remove systems to process components.
 *
 * World acts glue between various component managers and systems and abstracts away the internal
 * representations of the ECS architecture.
 */
class World
{
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
     *
     * Note: ComponentType must be default constructible
     */
    template <typename ComponentType>
    void addComponent(Entity entity, ComponentType comp = ComponentType())
    {
        ComponentManager<ComponentType> *manager = getComponentManager<ComponentType>();
        manager->add(entity, comp);
        updateEntityToSystem(entity, ComponentType::family(), true);
    }

    /**
     * Remove the component of type ComponentType from the entity
     * An easier approach is to get an EntityHandle from create entity and
     * call entity_handle.remove<ComponentType>();
     */
    template <typename ComponentType> void removeComponent(Entity entity)
    {
        ComponentManager<ComponentType> *manager = getComponentManager<ComponentType>();
        manager->remove(entity);
        updateEntityToSystem(entity, ComponentType::family(), false);
    }

    /**
     * Gets the component of type ComponentType from the entity
     * An easier approach is to get an EntityHandle from create entity and
     * call entity_handle.get<ComponentType>();
     */
    template <typename ComponentType> ComponentType *getComponent(Entity entity)
    {
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
     * Removes an entity that resides inside the given EntityHandle
     */
    void removeEntity(EntityHandle ent);

    /**
     * Check whether given entity is in this world
     * @param ent Entity
     * @return Whether exists in this world
     */
    bool hasEntity(EntityHandle ent) const;

    /**
     * Iterate through all existing entities using a range for loop
     * @return Proxy iterator for entities that will work with a range for loop
     */
    EntityHandleIterator<std::unordered_set<Entity>> entities();

    /**
     * Number of entities in the world
     * @return Count of entities
     */
    size_t numEntities() const;

    /**
     * Update the world (usually called in the game loop)
     */
    void update();

  protected:
    void updateEntityToSystem(Entity ent, int component_family, bool flag);

    template <typename ComponentType> ComponentManager<ComponentType> *getComponentManager()
    {
        auto it = component_mgrs_.find(ComponentType::family());
        if (it == component_mgrs_.end())
        {
            component_mgrs_[ComponentType::family()] =
                std::make_unique<ComponentManager<ComponentType>>();
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
 * This enables API like entity_handle.add(component) instead of world->addComponent(entity,
 * component)
 */
struct EntityHandle
{
    Entity entity;
    World *world;
    /**
     * Add a component to the entity
     * @param comp Component to add
     */
    template <typename T> void add(T comp = T())
    {
        world->addComponent(entity, comp);
    }
    /**
     * Remove the component of type T from the entity
     */
    template <typename T> void remove()
    {
        world->removeComponent<T>(entity);
    }
    /**
     * Get the component of type T from the entity
     * @return Pointer to the component which is actually stored in
     * the world's component manager
     */
    template <typename T> T *get()
    {
        return world->getComponent<T>(entity);
    }

    /**
     * Check whether this handle is valid
     * @return Whether valid
     */
    bool valid() const
    {
        return world != nullptr && world->hasEntity(*this);
    }
};

/**
 * EntityHandleIterator is a convenience class to wrap iterator like functionality
 * around Entities. Its main use is to return EntityHandles instead of raw Entitys.
 */
template <typename Container> class EntityHandleIterator
{
  public:
    EntityHandleIterator(World *world, Container &cnt)
    {
        world_ = world;
        curr_ = cnt.begin();
        end_ = cnt.end();
    }
    EntityHandle next()
    {
        return EntityHandle{*(curr_++), world_};
    }
    bool hasNext()
    {
        return curr_ != end_;
    }

  private:
    World *world_;
    typename Container::iterator curr_, end_;
};

} // namespace rcube

#endif // WORLD_H
