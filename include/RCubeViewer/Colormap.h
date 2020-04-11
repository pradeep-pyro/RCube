#pragma once

#include "glm/glm.hpp"
#include <vector>

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

void colormap(Colormap cm, float value, float vmin, float vmax, glm::vec3 &rgb);

void colormap(Colormap cm, const float *value, size_t size, float vmin, float vmax, glm::vec3 &rgb);

void colormap(Colormap cm, const std::vector<float> &value, float vmin, float vmax, std::vector<glm::vec3> &colors);

} // namespace rcube