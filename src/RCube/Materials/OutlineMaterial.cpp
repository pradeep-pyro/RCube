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
#ifdef RCUBE_OUTLINE_MATERIAL_NORMAL_OFFSET
    vec3 normal = normalize(normal);
    vec3 offset = normal * thickness;
    vec3 offset_position = position + offset;
    vec4 world_pos = model_matrix * vec4(offset_position, 1.0);
#else
    vec4 world_pos = model_matrix * vec4(position, 1.0);
#endif
    gl_Position = projection_matrix * view_matrix * world_pos;
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
    state_.stencil.write = true;
#ifdef RCUBE_OUTLINE_MATERIAL_POLYGON_OFFSET
    state_.polygon_mode = PolygonMode::Fill;
    state_.polygon_offset.enabled = true;
    state_.polygon_offset.offset = thickness;
#else
    state_.polygon_mode = PolygonMode::Line;
    state_.line_width = 2.f;
#endif

    shader_ = ShaderProgram::create(OutlineVertexShader, OutlineFragmentShader, true);
}

void OutlineMaterial::updateUniforms()
{
    shader_->uniform("color").set(color);
    // shader_->uniform("thickness").set(thickness);
#ifdef RCUBE_OUTLINE_MATERIAL_POLYGON_OFFSET
    state_.polygon_offset.offset = thickness;
#else
    state_.line_width = thickness;
#endif
}

void OutlineMaterial::drawGUI()
{
    ImGui::Text("OutlineMaterial");
    ImGui::Text("This material renders the outline of the object\nwith the inverted hull method.");
    ImGui::Separator();
    ImGui::ColorEdit3("Outline Color", glm::value_ptr(color),
                      ImGuiColorEditFlags_DisplayRGB | ImGuiColorEditFlags_DisplayHSV);
    ImGui::SliderFloat("Outline Thickness", &thickness, 2.f, 10.f);
}

} // namespace rcube