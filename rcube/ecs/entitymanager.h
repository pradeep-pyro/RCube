#ifndef ENTITYMANAGER_H
#define ENTITYMANAGER_H

#include "entity.h"
#include <vector>

/**
 * EntityManager creates new entities while ensuring that each one has a unique ID
 */
class EntityManager {
public:
    EntityManager() : last_id_(0) {}
    /**
     * Create a new entity with a unique ID
     * @return A new entity
     */
    Entity createEntity()  {
        return Entity{ last_id_++ };
    }
private:
    unsigned int last_id_;  /// Keep track of last assigned id to entity
};


#endif // ENTITYMANAGER_H
