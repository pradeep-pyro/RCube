#pragma once

#include "RCube/Core/Arch/Component.h"
#include <string>

namespace rcube
{

/**
 * A component to store a name for each entity
 */
class Name : public Component<Name>
{
  public:
    std::string name;

    Name() = default;
    Name(std::string val) : name(val)
    {
    }
};

} // namespace rcube
