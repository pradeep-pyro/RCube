#pragma once

#include "RCube/Core/Graphics/OpenGL/Effect.h"
namespace rcube
{

/**
 * GrayscaleEffect is a postprocessing effect that converts the contents of the screen
 * from RGB to grayscale while leaving the alpha channel untouched.
 * To use, simply add this effect to a Camera component:
 * entity.get<Camera>()->postprocess.push_back(makeGrayscaleEffect());
 */
std::shared_ptr<ShaderProgram> makeGrayscaleEffect();

} // namespace rcube
