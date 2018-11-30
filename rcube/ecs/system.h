#ifndef SYSTEM_H
#define SYSTEM_H

#include <vector>
#include <algorithm>
#include "entity.h"
#include <bitset>
#include <unordered_map>
#include <functional>

class World;

// typedef std::bitset<8> ComponentMask;


struct ComponentMask {
    std::bitset<8> bits;
    void set(size_t pos, bool flag=true);
    void reset(size_t pos);
    bool match(const ComponentMask &other);
    bool equal(const ComponentMask &other);
    std::string to_string() const;
};

bool operator==(const ComponentMask &lhs, const ComponentMask &rhs);

// custom specialization of std::hash can be injected in namespace std
namespace std
{
    template<> struct hash<ComponentMask> {
        typedef ComponentMask argument_type;
        typedef std::size_t result_type;
        result_type operator()(argument_type const& cm) const noexcept
        {
            return std::hash<std::bitset<8>>{}(cm.bits);
        }
    };
}

/*
struct ComponentMask {
    std::bitset<512> mask;
    bool match(const ComponentMask &other) {
        return mask == other.mask;
    }
    void set(size_t index, bool flag) {
        mask[index] = flag;
    }
    void set(std::vector<size_t> index, bool flag) {
        for (auto i : index) {
            mask[i] = flag;
        }
    }
};

namespace std {

template <>
struct hash<ComponentMask> {
    std::size_t operator()(const ComponentMask &k) const {
      return std::hash<std::bitset<512>>()(k.mask);
  }
};

}
*/

class System {
public:
    virtual ~System() = default;
    void addFilter(const ComponentMask &filter) {
        filters_.push_back(filter);
        registered_entities_[filter].reserve(512);
    }
    virtual void initialize() = 0;
    virtual void update(bool force) = 0;
    virtual void cleanup() = 0;
    virtual void registerWorld(World *world) {
        world_ = world;
    }
    /*virtual void registerEntity(const Entity &e) {
        registered_entities_.push_back(e);
    }
    virtual void unregisterEntity(const Entity &e) {
        registered_entities_.erase(
                std::remove_if(registered_entities_.begin(), registered_entities_.end(),
                    [&](const Entity &ent) { return ent.id == e.id; }),
            registered_entities_.end());
    }*/
    virtual void registerEntity(const Entity &e, ComponentMask sign) {
        registered_entities_[sign].push_back(e);
    }
    virtual void unregisterEntity(const Entity &e, ComponentMask sign) {
        std::vector<Entity> &entity_list = registered_entities_[sign];
        entity_list.erase(std::remove_if(entity_list.begin(), entity_list.end(),
        [&](const Entity &ent) {
            return ent.id == e.id;
        }),
        entity_list.end());
    }
    /*ComponentMask signature() const {
        return signature_;
    }*/
    const std::vector<ComponentMask> & filters() const {
        return filters_;
    }

protected:
    // std::vector<Entity> registered_entities_;
    std::unordered_map<ComponentMask, std::vector<Entity>> registered_entities_;
    // ComponentMask signature_;
    std::vector<ComponentMask> filters_;
    World *world_;
};

#endif // SYSTEM_H
