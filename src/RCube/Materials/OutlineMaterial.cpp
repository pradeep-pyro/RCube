#include "RCube/Materials/OutlineMaterial.h"
#include "RCube/Core/Graphics/ShaderManager.h"
#include "imgui.h"

namespace rcube
{
const static std::string OutlineVertexShader = R"(
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
out vec4 out_color;

uniform vec3 color;
uniform float opacity;

// Reference:
// https://digitalrune.github.io/DigitalRune-Documentation/html/fa431d48-b457-4c70-a590-d44b0840ab1e.htm
const mat4 bayerMatrix = mat4(
    vec4(1.0 / 17.0,  9.0 / 17.0,  3.0 / 17.0, 11.0 / 17.0),
    vec4(13.0 / 17.0,  5.0 / 17.0, 15.0 / 17.0,  7.0 / 17.0),
    vec4(4.0 / 17.0, 12.0 / 17.0,  2.0 / 17.0, 10.0 / 17.0),
    vec4(16.0 / 17.0,  8.0 / 17.0, 14.0 / 17.0,  6.0 / 17.0)
);

void main() {
    if (bayerMatrix[int(gl_FragCoord.x) % 4][int(gl_FragCoord.y) % 4] > opacity)
    {
        discard;
    }
    out_color = vec4(color, 1.0);
}
)";

OutlineMaterial::OutlineMaterial() : ShaderMaterial("OutlineMaterial")
{
    state_.blend.enabled = false;
    state_.blend.blend[0].color_src = BlendFunc::One;
    state_.blend.blend[0].color_dst = BlendFunc::One;

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

    ForwardRenderSystemShaderManager::instance().create("OutlineMaterial", OutlineVertexShader, OutlineFragmentShader, true);
}

void OutlineMaterial::updateUniforms(std::shared_ptr<ShaderProgram> shader)
{
    shader->uniform("color").set(color);
    shader->uniform("thickness").set(thickness);
    shader->uniform("opacity").set(std::max(0.f, std::min(1.f, opacity)));
}

void OutlineMaterial::drawGUI()
{
    ImGui::Text("OutlineMaterial");
    ImGui::Text("This material renders the outline of the object\nwith the inverted hull method.");
    ImGui::Separator();
    ImGui::ColorEdit3("Outline Color", glm::value_ptr(color),
                      ImGuiColorEditFlags_DisplayRGB | ImGuiColorEditFlags_DisplayHSV);
    ImGui::SliderFloat("Outline Thickness", &thickness, 0.01f, 1.f);
    ImGui::SliderFloat("Opacity", &opacity, 0.f, 1.f);
}

} // namespace rcube