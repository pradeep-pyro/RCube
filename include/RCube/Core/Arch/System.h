#ifndef SYSTEM_H
#define SYSTEM_H

#include "RCube/Core/Arch/Entity.h"
#include <algorithm>
#include <bitset>
#include <functional>
#include <unordered_map>
#include <vector>

#define RCUBE_COMPONENTMASK_BITS 32

namespace rcube
{
/**
 * ComponentMask is a bitset where bits at a particular position (from Component::family())
 * representing a component can be set. This is used to filter entities based on components of
 * interest and also update the ComponentManager when a component has been added or removed from an
 * entity
 */
struct ComponentMask
{
    ComponentMask() = default;
    template <typename... Ts> ComponentMask(size_t arg0, const Ts &... args)
    {
        std::vector<size_t> families{arg0, args...};
        for (auto &&x : families)
        {
            set(x);
        }
    }
    std::bitset<RCUBE_COMPONENTMASK_BITS> bits;
    void set(size_t pos, bool flag = true);
    void reset(size_t pos);
    bool match(const ComponentMask &other);
    bool equal(const ComponentMask &other);
    std::string to_string() const;
};

bool operator==(const ComponentMask &lhs, const ComponentMask &rhs);

} // namespace rcube

// custom specialization of std::hash for ComponentMask
namespace std
{
template <> struct hash<rcube::ComponentMask>
{
    typedef rcube::ComponentMask argument_type;
    typedef std::size_t result_type;
    result_type operator()(argument_type const &cm) const noexcept
    {
        return std::hash<std::bitset<RCUBE_COMPONENTMASK_BITS>>{}(cm.bits);
    }
};
} // namespace std

namespace rcube
{

class World;

/**
 * System is the base class for all systems such as TransformSystem, CameraSystem etc.
 * The goal of a system is to update the components of entities.
 * For example, a CameraSystem will use data from the Camera component to compute and set
 * the world-to-view and view-to-projection matrices.
 *
 * To implement a new system, derive from this class and implement initialize(), update() and
 * cleanup() To automatically obtain the list of entities that have certain combination of
 * components, use the addFilter() method to register them (multiple filters can exist in a single
 * System)
 */
class System
{
  public:
    virtual ~System() = default;
    /**
     * Add a component mask filter so that entities satisfying the filter are
     * registered to this system
     * @param filter Component mask filter to get entities of interest
     */
    void addFilter(const ComponentMask &filter)
    {
        filters_.push_back(filter);
        registered_entities_[filter].reserve(512);
    }
    /**
     * Gets a list of entities that match the given component mask filter
     * @param filter Component mask filter to get entities of interest
     * @return List of entities that were registered under te given filter
     */
    const std::vector<Entity> &getFilteredEntities(const ComponentMask &filter)
    {
        return registered_entities_[filter];
    }
    virtual void initialize()
    {
    }
    virtual void update(bool force) = 0;
    virtual void cleanup()
    {
    }
    virtual const std::string name() const = 0;
    /**
     * Register the world
     * @param world
     */
    virtual void registerWorld(World *world)
    {
        world_ = world;
    }
    /**
     * Register an entity so that it will be processed by this system
     * @param e Entity
     * @param sign Signature to classify this entity
     */
    virtual void registerEntity(const Entity &e, ComponentMask sign)
    {
        registered_entities_[sign].push_back(e);
    }
    /**
     * Unregister given entity from this system's registered entities with given signature
     * @param e Entity
     * @param sign Signature to classify this entity
     */
    virtual void unregisterEntity(const Entity &e, ComponentMask sign)
    {
        std::vector<Entity> &entity_list = registered_entities_[sign];
        entity_list.erase(std::remove_if(entity_list.begin(), entity_list.end(),
                                         [&](const Entity &ent) { return ent.id() == e.id(); }),
                          entity_list.end());
    }
    /**
     * List of filters that are used to handle entities with varying combinations of
     * components
     * @return List of filters
     */
    const std::vector<ComponentMask> &filters() const
    {
        return filters_;
    }

    virtual unsigned int priority() const = 0;

  protected:
    std::unordered_map<ComponentMask, std::vector<Entity>> registered_entities_;
    std::vector<ComponentMask> filters_;
    World *world_;
};

} // namespace rcube

#endif // SYSTEM_H
