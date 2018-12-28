#ifndef ENTITY_H
#define ENTITY_H

/**
 * Entity is some object in the virtual world
 * Internally it just stores a unique unsigned int ID.
 */
struct Entity {
    unsigned int id;
    bool operator==(const Entity &other) const {
        return id == other.id;
    }

    bool operator<(const Entity &other) const {
        return id < other.id;
    }
};

#endif // ENTITY_H
