#pragma once

#include "RCubeViewer/Colormap.h"
#include <string>
#include <vector>

namespace rcube
{
namespace viewer
{

class ScalarField
{
    friend class Pointcloud;
    std::vector<glm::vec3> colors_;
    std::vector<float> data_;
    Colormap cmap_ = Colormap::Viridis;
    float vmin_ = 0.f;
    float vmax_ = 1.f;
    bool dirty_ = true;

  public:
    const std::vector<float> &data() const
    {
        return data_;
    }
    std::vector<float> &data()
    {
        return data_;
    }
    void setData(const std::vector<float> &data)
    {
        data_ = data;
        dirty_ = true;
    }
    float dataMinRange() const
    {
        return vmin_;
    }
    void setDataMinRange(float val)
    {
        vmin_ = val;
        dirty_ = true;
    }
    float dataMaxRange() const
    {
        return vmax_;
    }
    void setDataMaxRange(float val)
    {
        vmax_ = val;
        dirty_ = true;
    }
    void fitDataRange()
    {
        auto minmax = std::minmax_element(std::begin(data_), std::end(data_));
        vmin_ = *minmax.first;
        vmax_ = *minmax.second;
        dirty_ = true;
    }
    Colormap cmap() const
    {
        return cmap_;
    }
    void setCmap(Colormap cmap)
    {
        cmap_ = cmap;
        dirty_ = true;
    }
    void updateColors()
    {
        if (dirty_)
        {
            colors_.clear();
            colormap(cmap_, data_, vmin_, vmax_, colors_);
            dirty_ = false;
        }
    }
};

} // namespace viewer
} // namespace rcube