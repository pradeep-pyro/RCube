#pragma once

#include "RCube/Core/Arch/Entity.h"
#include <algorithm>
#include <unordered_set>
#include <vector>

namespace std
{
template <> struct hash<rcube::Entity>
{
    size_t operator()(const rcube::Entity &ent) const
    {
        return hash<unsigned int>()(ent.id());
    }
};
} // namespace std

namespace rcube
{

/**
 * EntityManager manages the lifetime of entities.
 * It supports creation of new entities while ensuring that each one has a unique ID,
 * removal of entities (which are later reused), and iteration through existing entities.
 */
class EntityManager
{
  public:
    EntityManager() : last_id(0)
    {
    }
    /**
     * Create a new entity with a unique ID
     * @return A new entity
     */
    Entity createEntity()
    {
        Entity ent;
        // Reuse deleted entities if there are any
        if (deleted_entities.size() > 0)
        {
            ent = deleted_entities[deleted_entities.size() - 1];
            deleted_entities.pop_back();
        }
        else
        {
            // Otherwise, create a new entity
            ent = Entity(last_id++);
        }
        entities.insert(ent);
        return ent;
    }

    /**
     * Remove the given entity.
     * This entity will be reused in future.
     * @param ent Entity to be removed
     */
    void removeEntity(const Entity &ent)
    {
        // Check if entity exists
        auto ent_iter = entities.find(ent);
        // Return if entity does not exist
        if (ent_iter == entities.end())
        {
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
    bool hasEntity(const Entity &ent) const
    {
        return entities.find(ent) != entities.end();
    }

    size_t count() const
    {
        return entities.size();
    }

  public:
    std::unordered_set<Entity> entities;  /// Set of active entities
    unsigned int last_id;                 /// Keep track of last assigned id to entity
    std::vector<Entity> deleted_entities; /// List of deleted entities
};

} // namespace rcube
