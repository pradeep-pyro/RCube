#pragma once

#include "RCube/Core/Arch/Component.h"
#include "RCube/Window.h"

namespace rcube
{
namespace viewer
{

class Pickable : public Component<Pickable>
{
  public:
    bool active = true;
    bool picked = false;
    glm::vec3 point = glm::vec3(0.0, 0.0, 0.0);
    PrimitivePtr primitive = nullptr; // Primitive that was picked, e.g., triangle, sphere based on
                                      // what went into the BVH
};

} // namespace viewer
} // namespace rcube
