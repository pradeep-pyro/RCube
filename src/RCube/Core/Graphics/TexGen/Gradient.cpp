#include "RCube/Core/Graphics/TexGen/Gradient.h"
#include <iostream>

namespace rcube
{

Image gradientV(int width, int height, glm::vec3 color_top, glm::vec3 color_bottom, float exponent)
{
    Image im;
    std::vector<unsigned char> data;
    data.reserve(width * height * 4 /* RGBA */);
    for (int row = 0; row < height; ++row)
    {
        float t = static_cast<float>(row) / static_cast<float>(height - 1) * 2.f - 1.f;
        t = std::pow(std::min(1.0f, 1.0f - t), exponent);
        for (int col = 0; col < width; ++col)
        {
            glm::vec3 color = t * color_top + (1.f - t) * color_bottom;
            data.push_back((unsigned char)(color.r * 255));
            data.push_back((unsigned char)(color.g * 255));
            data.push_back((unsigned char)(color.b * 255));
            data.push_back(255); // Alpha
        }
    }
    im.setPixels(width, height, 4, data);
    return im;
}

} // namespace rcube