#include "RCubeViewer/ScalarField.h"
#include "RCubeViewer/Colormap.h"
#include <algorithm>
#include <string>
#include <vector>

namespace rcube
{
namespace viewer
{
const std::vector<float> &ScalarField::data() const
{
    return data_;
}
std::vector<float> &ScalarField::data()
{
    return data_;
}
void ScalarField::setData(const std::vector<float> &data)
{
    data_ = data;
    dirty_ = true;
}
float ScalarField::dataMinRange() const
{
    return vmin_;
}
void ScalarField::setDataMinRange(float val)
{
    vmin_ = val;
    dirty_ = true;
}
float ScalarField::dataMaxRange() const
{
    return vmax_;
}
void ScalarField::setDataMaxRange(float val)
{
    vmax_ = val;
    dirty_ = true;
}
void ScalarField::fitDataRange()
{
    auto minmax = std::minmax_element(std::begin(data_), std::end(data_));
    vmin_ = *minmax.first;
    vmax_ = *minmax.second;
    dirty_ = true;
}
Colormap ScalarField::cmap() const
{
    return cmap_;
}
void ScalarField::setCmap(Colormap cmap)
{
    cmap_ = cmap;
    dirty_ = true;
}
void ScalarField::updateColors()
{
    if (dirty_)
    {
        colors_.clear();
        colormap(cmap_, data_, vmin_, vmax_, colors_);
        dirty_ = false;
    }
}

} // namespace viewer
} // namespace rcube