#include "RCube/Materials/DepthMaterial.h"
#include "glm/gtc/type_ptr.hpp"
#include "imgui.h"

namespace rcube
{

const static std::string DepthVertexShader = R"(
#version 450
layout (location = 0) in vec3 position;

layout (std140, binding=0) uniform Camera {
    mat4 view_matrix;
    mat4 projection_matrix;
    mat4 viewport_matrix;
    vec3 eye_pos;
};

out vec3 vert_position;

uniform mat4 model_matrix;

void main()
{
    vec4 world_pos = model_matrix * vec4(position, 1.0);
    vert_position = world_pos.xyz;
    gl_Position = projection_matrix * view_matrix * world_pos;
}
)";

const static std::string DepthFragmentShader = R"(
#version 450
in vec3 vert_position;
out vec4 out_color;

uniform float znear, zfar;

// Based on: https://learnopengl.com/Advanced-OpenGL/Depth-testing
float linearDepth(float depth)
{
    float ndc_depth = 2.0 * depth - 1.0;
    return (2.0 * znear * zfar) / (zfar + znear - ndc_depth * (zfar - znear));
}

void main() {
    float depth = linearDepth(gl_FragCoord.z) / zfar;
    out_color = vec4(depth, depth, depth, 1.0);
}
)";

DepthMaterial::DepthMaterial()
{
    shader_ = ShaderProgram::create(DepthVertexShader, DepthFragmentShader, true);
}

void DepthMaterial::updateUniforms()
{
    shader_->uniform("znear").set(znear);
    shader_->uniform("zfar").set(zfar);
}

void DepthMaterial::drawGUI()
{
    ImGui::Text("DepthMaterial");
    ImGui::Text("This material renders the linear depth\nof the object in clip space");
    ImGui::Separator();
    ImGui::InputFloat("Near", &znear);
    ImGui::InputFloat("Far", &zfar);
}

} // namespace rcube