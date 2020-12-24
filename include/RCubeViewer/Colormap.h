#pragma once

#include "glm/glm.hpp"
#include <vector>

namespace rcube
{

enum class Colormap
{
    Viridis,
    Magma,
};

void colormap(Colormap cm, float value, float vmin, float vmax, glm::vec3 &rgb);

void colormap(Colormap cm, const float *value, size_t size, float vmin, float vmax, std::vector<glm::vec3> &rgb);

void colormap(Colormap cm, const std::vector<float> values, float vmin, float vmax,
              std::vector<float> &colors);

void colormap(Colormap cm, const std::vector<float> &value, float vmin, float vmax, std::vector<glm::vec3> &colors);

} // namespace rcube