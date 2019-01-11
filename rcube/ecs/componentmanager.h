#ifndef COMPONENTMANAGER_H
#define COMPONENTMANAGER_H

#include <map>
#include <array>
#include <iostream>
#include <vector>
#include "entity.h"

namespace rcube {

/**
 * Base class for all component managers.
 * For internal use only.
 */
class BaseComponentManager {
public:
    virtual ~BaseComponentManager() = default;
    virtual bool has(Entity e) const = 0;
    virtual void remove(Entity e) = 0;
};

/**
 * ComponentManager stores the component of type T corresponding to all entities created
 */
template <typename T>
class ComponentManager : public BaseComponentManager {
public:
    typedef unsigned int ComponentIndex;
    virtual ~ComponentManager() = default;
    ComponentManager() {
        component_data_.data.resize(1024);
    }
    /**
     * Add a component of type T to the given entity
     * @param e Entity to add component to
     * @param component Component to be added
     */
    void add(Entity e, const T &component) {
        ComponentIndex new_index = ComponentIndex{ component_data_.size };
        component_data_.size += 1;
        entity_map_[e] = new_index;
        component_data_.data[new_index] = component;
        // return new_index;
    }
    /**
     * Remove the component of type T from the given entity
     * @param e Entity to add component to
     */
    void remove(Entity e) override {
        if (entity_map_.find(e) == entity_map_.end()) {
            return;
        }
        ComponentIndex to_remove = entity_map_[e];
        component_data_.data[to_remove] = component_data_.data[component_data_.size - 1];
        entity_map_.erase(e);
        component_data_.size -= 1;
    }

    /**
     * Whether the given entity has a component of this type
     * @param e Entity
     * @return Whether entity has this component
     */
    bool has(Entity e) const override {
        return entity_map_.find(e) != entity_map_.end();
    }
    /**
     * Clears all components and entities from the manager
     */
    void clear() {
        entity_map_.clear();
        component_data_.data.clear();
        component_data_.size = 0;
    }
    /**
     * Get a pointer to the component of type T in the given entity
     * @param e Entity
     * @return Pointer to component of type T
     */
    T * get(Entity e) {
        if (entity_map_.find(e) == entity_map_.end()) {
            // throw std::runtime_error("Entity does not have requested component");
            std::cerr << "Entity does not have requested component" << std::endl;
            return nullptr;
        }
        return &(component_data_.data[entity_map_[e]]);
    }

private:
    struct ComponentData {
        unsigned int size = 1;
        //std::array<T, 1024> data;
        std::vector<T> data;
    };
    std::map<Entity, ComponentIndex> entity_map_;
    ComponentData component_data_;
};

} // namespace rcube

#endif // COMPONENTMANAGER_H
