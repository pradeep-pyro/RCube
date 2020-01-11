#pragma once

#include "RCube/Core/Arch/Component.h"
#include <string>

namespace rcube
{

class ScalarField : public Component<ScalarField>
{
  public:
    static constexpr int ATTRIBUTE_LOCATION =
        5; // Add a custom attribute to the Mesh at this location to get detected as a scalar field
    enum Colormap
    {
        Viridis = 0,
        Plasma = 1,
        Magma = 2,
        Inferno = 3,
        Jet = 4
    };
    bool show = true;
    float vmin = 0.0;
    float vmax = 1.0;
    Colormap colormap = Colormap::Viridis;
};

} // namespace rcube