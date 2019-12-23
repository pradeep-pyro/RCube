#include "RCube/Core/Graphics/Materials/FlatMaterial.h"
#include <string>

namespace rcube
{

const std::string vert_src = R"(
#version 420

layout (location = 0) in vec3 vertex;
layout (location = 3) in vec3 color;

layout (std140, binding=0) uniform Matrices {
    mat4 view_matrix;
    mat4 projection_matrix;
    mat4 viewport_matrix;
};

uniform mat4 model_matrix;

out vec3 frag_color;

void main() {
    vec4 world_vertex = model_matrix * vec4(vertex, 1.0);
    gl_Position = projection_matrix * view_matrix * world_vertex;
    frag_color = color;
}
)";

const std::string frag_src = R"(
#version 420

in vec3 frag_color;
out vec4 out_color;

void main() {
    out_color = vec4(frag_color, 1.0);
}
)";

FlatMaterial::FlatMaterial()
{
    render_settings.blending = false;
    render_settings.depth_write = true;
    render_settings.depth_test = true;
    render_settings.culling = false;
    initialize();
}
std::string FlatMaterial::vertexShader()
{
    return vert_src;
}
std::string FlatMaterial::fragmentShader()
{
    return frag_src;
}
std::string FlatMaterial::geometryShader()
{
    return "";
}
void FlatMaterial::setUniforms()
{
}

int FlatMaterial::renderPriority() const
{
    return RenderPriority::Opaque;
}

} // namespace rcube
