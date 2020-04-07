#pragma once

#include "glm/glm.hpp"

namespace rcube
{

enum class Colormap
{
    Viridis = 0,
    Plasma = 1,
    Magma = 2,
    Inferno = 3,
    Jet = 4,
    Grays = 5
};

void colormap(Colormap cm, float value, glm::vec3 &rgb);

} // namespace rcube