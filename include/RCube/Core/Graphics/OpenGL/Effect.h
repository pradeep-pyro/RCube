#pragma once

#include "RCube/Core/Graphics/OpenGL/Framebuffer.h"
#include "RCube/Core/Graphics/OpenGL/Mesh.h"
#include "RCube/Core/Graphics/OpenGL/ShaderProgram.h"
#include <memory>

namespace rcube
{

/**
 * An Effect is a image based post processing shader applied to the framebuffer's color texture.
 * To implement a new effect, create a fragment shader and pass it as argument.
 */
std::shared_ptr<ShaderProgram> makeEffect(const std::string &fragment_shader);

} // namespace rcube
