#ifndef ENTITY_H
#define ENTITY_H

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
