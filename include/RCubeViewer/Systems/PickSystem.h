#pragma once

#include "RCube/Core/Arch/System.h"
#include "glm/glm.hpp"

namespace rcube
{
namespace viewer
{

/**
 * The PickSystem is used to select objects in the scene using the left mouse button.
 * It works by casting a ray from the camera that is controlled by the user (indicated entity that has
 * Camera and CameraController components), onto every entity that has Drawable, Transform and Pickable
 * components.
 */
class PickSystem : public System
{
  public:
    PickSystem();
    virtual ~PickSystem() = default;
    virtual void update(bool) override;
    virtual unsigned int priority() const override;
    virtual const std::string name() const override;
};

} // namespace viewer
} // namespace rcube