#ifndef ENTITYMANAGER_H
#define ENTITYMANAGER_H

#include <algorithm>
#include "entity.h"
#include <vector>
#include <unordered_set>

/**
 * EntityManager creates new entities while ensuring that each one has a unique ID
 */
#ifdef TEST123
class EntityManager {
public:
    EntityManager() : last_id_(0) {
        entities_.reserve(1024);
    }
    /**
     * Create a new entity with a unique ID
     * @return A new entity
     */
    Entity createEntity() {
        auto ent = Entity(last_id_++);
        entities_.push_back(ent);
        return ent;
    }

    bool hasEntity(const Entity &ent) {
        return std::find(entities_.begin(), entities_.end(), ent) != entities_.end();
    }
private:
    unsigned int last_id_;  /// Keep track of last assigned id to entity
    std::vector<Entity> entities_;
};
#endif

namespace std {
template<>
struct hash<Entity> {
    size_t operator()(const Entity &ent) const {
        return hash<unsigned int>()(ent.id());
    }
};
}

/**
 * EntityManager manages the lifetime of entities.
 * It supports creation of new entities while ensuring that each one has a unique ID,
 * removal of entities (which are later reused), and iteration through existing entities.
 */
class EntityManager {
public:
    EntityManager() : last_id(0) {
    }
    /**
     * Create a new entity with a unique ID
     * @return A new entity
     */
    Entity createEntity() {
        // Reuse deleted entities if there are any
        if (deleted_entities.size() > 0) {
            Entity ent = deleted_entities[deleted_entities.size() - 1];
            deleted_entities.pop_back();
            return ent;
        }
        // Otherwise, create a new entity
        Entity ent = Entity(last_id++);
        entities.insert(ent);
        return ent;
    }

    /**
     * Remove the given entity.
     * This entity will be reused in future.
     * @param ent Entity to be removed
     */
    void removeEntity(const Entity &ent) {
        // Check if entity exists
        auto ent_iter = entities.find(ent);
        // Return if entity does not exist
        if (ent_iter == entities.end()) {
            return;
        }
        // Move entity to deleted list for reusing in future
        deleted_entities.push_back(*ent_iter);
        // Delete entity from entities list
        entities.erase(ent_iter);
    }

    /**
     * Check whether an entity exists
     * @param ent Entity
     * @return Whether exists
     */
    bool hasEntity(const Entity &ent) const{
        return entities.find(ent) != entities.end();
    }

    size_t count() const {
        return entities.size();
    }

public:
    std::unordered_set<Entity> entities;  /// Set of active entities
    unsigned int last_id;  /// Keep track of last assigned id to entity
    std::vector<Entity> deleted_entities; /// List of deleted entities
};


#endif // ENTITYMANAGER_H
