#include "RCube/Core/Graphics/OpenGL/Effect.h"

namespace rcube
{

// This is the default vertex shader for all effects
const static VertexShader EFFECT_VERTEX_SHADER({ShaderAttributeDesc{"vertex", GLDataType::Vec3f},
                                                ShaderAttributeDesc{"texcoord", GLDataType::Vec2f}},
                                               {}, R"(
#version 420
layout (location = 0) in vec3 vertex;
layout (location = 2) in vec2 texcoord;
out vec2 v_texcoord;

void main() {
    v_texcoord = texcoord;
    gl_Position = vec4(vertex, 1.0);
}
)");

std::shared_ptr<ShaderProgram> makeEffect(const FragmentShader &fragment_shader)
{
    return ShaderProgram::create(EFFECT_VERTEX_SHADER, fragment_shader, true);
}

} // namespace rcube
