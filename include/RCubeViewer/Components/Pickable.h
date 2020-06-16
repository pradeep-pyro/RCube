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
    size_t triangle = 0;
};

} // namespace viewer
} // namespace rcube
