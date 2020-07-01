#include "RCube/Core/Graphics/OpenGL/Effect.h"

namespace rcube
{

// This is the default vertex shader for all effects
const static std::string EffectVertexShader =  R"(
#version 420
layout (location = 0) in vec3 vertex;
layout (location = 2) in vec2 texcoord;
out vec2 v_texcoord;

void main() {
    v_texcoord = texcoord;
    gl_Position = vec4(vertex, 1.0);
}
)";

std::shared_ptr<ShaderProgram> makeEffect(const std::string &fragment_shader)
{
    return ShaderProgram::create(EffectVertexShader, fragment_shader, true);
}

} // namespace rcube
