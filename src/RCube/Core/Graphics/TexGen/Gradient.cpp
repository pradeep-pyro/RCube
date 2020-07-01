#include "RCube/Core/Graphics/TexGen/Gradient.h"

namespace rcube
{

Image gradientV(int width, int height, glm::vec3 color_top, glm::vec3 color_bottom)
{
    Image im;
    std::vector<unsigned char> data;
    data.reserve(width * height * 4 /* RGBA */);
    for (int row = 0; row < height; ++row)
    {
        const float t = static_cast<float>(row) / static_cast<float>(height - 1);
        for (int col = 0; col < width; ++col)
        {
            glm::vec3 color = (1.f - t) * color_top + t * color_bottom;
            data.push_back(color.r * 255);
            data.push_back(color.g * 255);
            data.push_back(color.b * 255);
            data.push_back(255); // Alpha
        }
    }
    im.setPixels(width, height, 4, data);
    return im;
}

} // namespace rcube