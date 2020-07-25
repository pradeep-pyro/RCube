#include "RCube/Core/Graphics/Materials/FlatMaterial.h"
#include <string>
#include "imgui.h"

namespace rcube
{

const static std::string FlatVertexShader = R"(
#version 420

layout (location = 0) in vec3 vertex;
layout (location = 3) in vec3 color;

layout (std140, binding=0) uniform Camera {
    mat4 view_matrix;
    mat4 projection_matrix;
    mat4 viewport_matrix;
    vec3 eye_pos;
};

uniform mat4 model_matrix;

out vec3 frag_color;

void main() {
    vec4 world_vertex = model_matrix * vec4(vertex, 1.0);
    gl_Position = projection_matrix * view_matrix * world_vertex;
    frag_color = color;
}
)";

const static std::string FlatFragmentShader = R"(
#version 420

in vec3 frag_color;
out vec4 out_color;

void main() {
    out_color = vec4(frag_color, 1.0);
}
)";

FlatMaterial::FlatMaterial()
{
    shader_ = ShaderProgram::create(FlatVertexShader, FlatFragmentShader, true);
}

void FlatMaterial::setUniforms()
{
}

void FlatMaterial::drawGUI()
{
    ImGui::Text("Flat Material");
    ImGui::Text("This material directly renders per-vertex colors without any shading");
}

const RenderSettings FlatMaterial::renderState() const
{
    RenderSettings state;
    state.blend.enabled = false;
    state.depth.write = true;
    state.depth.test = true;
    state.cull.enabled = false;
    return state;
}

} // namespace rcube
