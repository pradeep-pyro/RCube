#ifndef ENTITYMANAGER_H
#define ENTITYMANAGER_H

#include "entity.h"
#include <vector>

class EntityManager {
public:
    EntityManager() : last_id_(0) {}
    Entity createEntity()  {
        return Entity{ last_id_++ };
    }
private:
    unsigned int last_id_;
};


#endif // ENTITYMANAGER_H
