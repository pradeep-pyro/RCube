#include "RCube/Materials/UnlitMaterial.h"
#include "glm/gtc/type_ptr.hpp"
#include "imgui.h"

namespace rcube
{

const static std::string UnlitVertexShader = R"(
#version 450
layout (location = 0) in vec3 position;
layout (location = 3) in vec3 color;

layout (std140, binding=0) uniform Camera {
    mat4 view_matrix;
    mat4 projection_matrix;
    mat4 viewport_matrix;
    vec3 eye_pos;
};

out vec3 vert_position;
out vec3 vert_color;

uniform mat4 model_matrix;

void main()
{
    vec4 world_pos = model_matrix * vec4(position, 1.0);
    vert_position = world_pos.xyz;
    vert_color = pow(color, vec3(2.2));
    gl_Position = projection_matrix * view_matrix * world_pos;
}
)";

const static std::string UnlitFragmentShader = R"(
#version 450
in vec3 vert_position;
in vec3 vert_color;
out vec4 out_color;

uniform vec3 color;
uniform bool use_vertex_colors;
uniform float opacity;

void main() {
    vec3 result = use_vertex_colors ? vert_color : color;
    out_color = vec4(result, opacity);
}
)";

UnlitMaterial::UnlitMaterial()
{
    state_.blend.enabled = true;
    state_.cull.enabled = false;
    state_.depth.test = true;
    state_.depth.write = true;
    state_.depth.func = DepthFunc::Less;
    state_.dither = false;
    state_.stencil.test = true;
    state_.stencil.write = true;

    shader_ = ShaderProgram::create(UnlitVertexShader, UnlitFragmentShader, true);
}

void UnlitMaterial::updateUniforms()
{
    shader_->uniform("color").set(color);
    shader_->uniform("opacity").set(opacity);
    shader_->uniform("use_vertex_colors").set(use_vertex_colors);
}

void UnlitMaterial::drawGUI()
{
    ImGui::Text("UnlitMaterial");
    ImGui::Text("This material renders the object with a\nsingle color or per-vertex colors");
    ImGui::Separator();
    ImGui::ColorEdit3("Color", glm::value_ptr(color),
                      ImGuiColorEditFlags_DisplayRGB | ImGuiColorEditFlags_DisplayHSV);
    ImGui::Checkbox("Use vertex colors", &use_vertex_colors);
    ImGui::SliderFloat("Opacity", &opacity, 0.f, 1.f);
}

} // namespace rcube