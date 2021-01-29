#include "RCubeViewer/ScalarField.h"
#include "RCube/Core/Graphics/MeshGen/Points.h"
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
void ScalarField::setData(const std::vector<float> &data)
{
    data_ = data;
    updateHistogram();
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
bool ScalarField::updateColors()
{
    if (dirty_)
    {
        colormap(cmap_, data_, vmin_, vmax_, colors_);
        dirty_ = false;
        return true;
    }
    return false;
}

void ScalarField::updateHistogram()
{
    auto minmax = std::minmax_element(std::begin(data_), std::end(data_));
    float vmin = *minmax.first;
    float vmax = *minmax.second;
    const size_t bins = 10;
    float step = (vmax - vmin) / float(bins);
    std::vector<std::pair<float, float>> bin_ranges;
    for (size_t i = 0; i < bins; ++i)
    {
        bin_ranges.push_back({vmin + step * float(i), vmin + step * float(i + 1)});
    }
    histogram_.resize(bins, 0.f);
    float sum = 0.f;
    for (float val : data_)
    {
        for (size_t ii = 0; ii < bin_ranges.size(); ++ii)
        {
            const auto &bin_range = bin_ranges[ii];
            if (val >= bin_range.first && val < bin_range.second)
            {
                histogram_[ii] += 1.f;
                sum += 1.f;
                break;
            }
        }
    }
    if (sum > 0.f)
    {
        for (float &val : histogram_)
        {
            val /= sum;
        }
    }
}

} // namespace viewer
} // namespace rcube