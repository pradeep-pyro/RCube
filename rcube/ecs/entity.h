#ifndef ENTITY_H
#define ENTITY_H

namespace rcube {

/**
 * Entity is some object in the virtual world
 * Internally it just stores a unique unsigned int ID.
 */
struct Entity {
    Entity() = default;
    Entity(unsigned int id) : id_(id) {}
    unsigned int id() const {
        return id_;
    }
    bool operator==(const Entity &other) const {
        return id_ == other.id_;
    }

    bool operator<(const Entity &other) const {
        return id_ < other.id_;
    }
private:
    unsigned int id_;  /// Unique id of each entity
};

} // namespace rcube

#endif // ENTITY_H
