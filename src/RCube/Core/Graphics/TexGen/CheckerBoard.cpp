#include "RCube/Core/Graphics/TexGen/CheckerBoard.h"
namespace rcube
{

Image checkerboard(int width, int height, int checker_width, int checker_height, glm::vec3 color1,
                   glm::vec3 color2)
{
    Image im;
    std::vector<unsigned char> data;
    data.reserve(width * height * 4 /* RGBA */);
    for (int row = 0; row < height; ++row)
    {
        int checker_row = row / checker_height;
        for (int col = 0; col < width; ++col)
        {
            int checker_col = col / checker_width;
            const glm::vec3 &color = (((checker_row + checker_col) % 2) == 0) ? color1 : color2;
            data.push_back(unsigned char(color.r * 255));
            data.push_back(unsigned char(color.g * 255));
            data.push_back(unsigned char(color.b * 255));
            data.push_back(255); // Alpha
        }
    }
    im.setPixels(width, height, 4, data);
    return im;
}

} // namespace rcube
