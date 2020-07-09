#pragma once

#include "RCube/Core/Graphics/OpenGL/Image.h"
#include "glm/glm.hpp"

namespace rcube
{

/**
 * Returns an image depicting a vertical gradient texture
 * @param width Width of the texture
 * @param height Height of the texture
 * @param color_top Color at the top row of the image
 * @param color_bottom Color at the bottom row of the image
 * @param exponent Exponent value to harden the border between two colors
 * @return
 */
Image gradientV(int width, int height, glm::vec3 color_top, glm::vec3 color_bottom, float exponent=1.f);

} // namespace rcube