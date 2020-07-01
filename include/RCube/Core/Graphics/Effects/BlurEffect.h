#pragma once

#include "RCube/Core/Graphics/OpenGL/Effect.h"
#include <memory>

namespace rcube
{

/**
 * BlurEffect is a postprocessing effect that blurs the contents of the screen
 * using a 9-tap Gaussian filter
 * To use, simply add this effect to a Camera component:
 * entity.get<Camera>()->postprocess.push_back(makeBlurEffect(...));
 */
std::shared_ptr<ShaderProgram> makeBlurEffect(bool horizonal);

} // namespace rcube
