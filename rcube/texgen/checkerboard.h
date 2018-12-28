#ifndef CHECKERBOARD_H
#define CHECKERBOARD_H

#include "glm/glm.hpp"
#include "../render/Image.h"

namespace rcube {

/**
 * Returns an image depicting a checkerboard texture
 * @param width Width of the texture
 * @param height Height of the texture
 * @param checker_width Width of each box in the checkerboard
 * @param checker_height Height of each box in the checkerboard
 * @param color1 Color 1
 * @param color2 Color 2
 * @return
 */
Image checkerboard(int width, int height, int checker_width, int checker_height,
                   glm::vec3 color1, glm::vec3 color2);

} // namespace rcube

#endif // CHECKERBOARD_H
