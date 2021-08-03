#include "RCube/Materials/UnlitMaterial.h"
#include "RCube/Core/Graphics/ShaderManager.h"
#include "glm/gtc/type_ptr.hpp"
#include "imgui.h"

namespace rcube
{

const static std::string UnlitVertexShader = R"(
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
in vec3 vert_position;
in vec3 vert_color;

#if RCUBE_RENDERPASS == 0
out vec4 out_color;
#elif RCUBE_RENDERPASS == 1
layout (location = 0) out vec4 accum;
layout (location = 1) out float reveal;
#endif

uniform vec3 color;
uniform bool use_vertex_colors;
uniform float opacity;

void main() {
    vec3 result = use_vertex_colors ? vert_color : color;
    vec4 final_color = vec4(result, opacity);
#if RCUBE_RENDERPASS == 0
    out_color = final_color;
#elif RCUBE_RENDERPASS == 1
    // Based on https://learnopengl.com/Guest-Articles/2020/OIT/Weighted-Blended
    // and http://casual-effects.blogspot.com/2015/03/implemented-weighted-blended-order.html
    float a = min(1.0, final_color.a) * 8.0 + 0.01;
    float b = -gl_FragCoord.z * 0.95 + 1.0;
    float weight = clamp(a * a * a * 1e8 * b * b * b, 1e-2, 3e2);
    accum = vec4(final_color.rgb * final_color.a, final_color.a) * weight;
    reveal = final_color.a;
#endif
}
)";

UnlitMaterial::UnlitMaterial() : ShaderMaterial("UnlitMaterial")
{
    ForwardRenderSystemShaderManager::instance().create("UnlitMaterial", UnlitVertexShader,
                                                        UnlitFragmentShader, true);
}

void UnlitMaterial::updateUniforms(std::shared_ptr<ShaderProgram> shader)
{
    shader->uniform("color").set(color);
    shader->uniform("use_vertex_colors").set(use_vertex_colors);
    ShaderMaterial::updateUniforms(shader);
}

void UnlitMaterial::drawGUI()
{
    ImGui::Text("UnlitMaterial");
    ImGui::Text("This material renders the object with a\nsingle color or per-vertex colors");
    ImGui::Separator();
    ImGui::ColorEdit3("Color", glm::value_ptr(color),
                      ImGuiColorEditFlags_DisplayRGB | ImGuiColorEditFlags_DisplayHSV);
    ShaderMaterial::drawGUI();
    ImGui::Checkbox("Use vertex colors", &use_vertex_colors);
}

} // namespace rcube