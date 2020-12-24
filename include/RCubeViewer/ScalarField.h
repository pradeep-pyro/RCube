#pragma once

#include <string>
#include <vector>
#include "RCubeViewer/Colormap.h"

namespace rcube
{
namespace viewer
{

class ScalarField
{
    friend class Pointcloud;
    std::vector<glm::vec3> colors_;
  public:
    std::vector<float> data;
    float vmin = 0.f;
    float vmax = 1.f;
    Colormap colormap = Colormap::Viridis;
};

} // namespace viewer
} // namespace rcube