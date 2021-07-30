#pragma once

#include <functional>

#include "RCube/Core/Arch/System.h"
#include "RCube/Core/Arch/World.h"
#include "RCube/Core/Graphics/OpenGL/Buffer.h"
#include "glm/glm.hpp"

namespace rcube
{

class ForwardRenderSystem;

namespace viewer
{

/**
 * Helper class to manage ping-pong buffers (or any objects)
 */
template <class T> class PingPongHelper
{
    T objects_[2];
    int index_ = 0;

  public:
    T &first()
    {
        return objects_[index_];
    }
    const T &first() const
    {
        return objects_[index_];
    }
    T &second()
    {
        return objects_[1 - index_];
    }
    const T &second() const
    {
        return objects_[1 - index_];
    }
    void increment()
    {
        index_ = 1 - index_;
    }
};

/**
 * The PickSystem is used to select objects in the scene using the left mouse button.
 * It works by casting a ray from the camera that is controlled by the user (indicated entity that
 * has Camera and CameraController components), onto every entity that has Drawable, Transform and
 * Pickable components.
 */
class PickSystem : public System
{
    ForwardRenderSystem *render_system_ = nullptr;
    PingPongHelper<std::shared_ptr<PixelPackBuffer>> pbos_;
    std::function<bool()> criterion_;
  public:
    PickSystem();
    virtual ~PickSystem() = default;
    virtual void update(bool) override;
    virtual unsigned int priority() const override;
    virtual const std::string name() const override;
    void setCriterion(std::function<bool()> crit);
    void pickPoint(glm::ivec2 coord, EntityHandle &picked_entity, size_t &picked_primitive_index);
    void pickRectangle(glm::ivec2 origin, glm::ivec2 size,
                       std::vector<std::pair<EntityHandle, std::vector<size_t>>> &picked);
};

} // namespace viewer
} // namespace rcube