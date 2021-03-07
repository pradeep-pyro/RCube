#include "RCube/Materials/OutlineMaterial.h"
#include "imgui.h"

namespace rcube
{
const static std::string OutlineVertexShader = R"(
#version 450
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (std140, binding=0) uniform Camera {
    mat4 view_matrix;
    mat4 projection_matrix;
    mat4 viewport_matrix;
    vec3 eye_pos;
};

uniform mat4 model_matrix;

uniform float thickness;

void main()
{
    mat4 mvp = projection_matrix * view_matrix * model_matrix;
    vec4 clip_position = mvp * vec4(position, 1.0);
    vec4 vp_position = viewport_matrix * clip_position;
    vec3 vp_normal = vec3(mvp * vec4(normal, 0.0));
    vp_position.xy += normalize(vp_normal.xy) * thickness * vp_position.w;
    gl_Position = inverse(viewport_matrix) * vp_position;
}
)";

const static std::string OutlineFragmentShader = R"(
#version 450
out vec4 out_color;

uniform vec3 color;

void main() {
    out_color = vec4(color, 1.0);
}
)";
 
OutlineMaterial::OutlineMaterial()
{
    state_.blend.enabled = false;
    state_.blend.color_src = BlendFunc::One;
    state_.blend.color_dst = BlendFunc::One;

    // Frontface culling is important to render the outline
    // using the inverted-hull method that is used here
    state_.cull.enabled = true;
    state_.cull.mode = Cull::Front;
    state_.depth.test = true;
    state_.depth.write = true;
    state_.depth.func = DepthFunc::Less;
    state_.dither = false;
    state_.stencil.test = true;
    state_.stencil.write = 0x00;
    state_.stencil.func = StencilFunc::NotEqual;
    state_.stencil.func_ref = 1;
    state_.stencil.func_mask = 0xFF;

    shader_ = ShaderProgram::create(OutlineVertexShader, OutlineFragmentShader, true);
}

void OutlineMaterial::updateUniforms()
{
    shader_->uniform("color").set(color);
     shader_->uniform("thickness").set(thickness);
}

void OutlineMaterial::drawGUI()
{
    ImGui::Text("OutlineMaterial");
    ImGui::Text("This material renders the outline of the object\nwith the inverted hull method.");
    ImGui::Separator();
    ImGui::ColorEdit3("Outline Color", glm::value_ptr(color),
                      ImGuiColorEditFlags_DisplayRGB | ImGuiColorEditFlags_DisplayHSV);
    ImGui::SliderFloat("Outline Thickness", &thickness, 0.01f, 1.f);
}

} // namespace rcube