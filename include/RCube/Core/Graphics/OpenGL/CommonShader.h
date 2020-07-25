#pragma once

#include <string>
#include <memory>
#include "RCube/Core/Graphics/OpenGL/ShaderProgram.h"

namespace rcube
{
namespace common
{

/**
 * Skybox vertex shader
 * This shader works with the rcube::common::skyboxMesh() mesh and
 * renders a skybox at depth = 1.0
 */
const static std::string SKYBOX_VERTEX_SHADER = R"(
#version 420

layout (location = 0) in vec3 position;
out vec3 texcoords;

layout (std140, binding=0) uniform Camera {
    mat4 view_matrix;
    mat4 projection_matrix;
    mat4 viewport_matrix;
    vec3 eye_pos;
};

void main() {
    texcoords = position;
    vec4 pos = projection_matrix * mat4(mat3(view_matrix)) * vec4(position, 1.0);
    gl_Position = pos.xyww;
}
)";

/**
 * Skybox fragment shader
 * This shader works with the rcube::common::skyboxMesh() mesh and
 * renders a skybox at depth = 1.0
 */
const static std::string SKYBOX_FRAGMENT_SHADER = R"(
#version 420

out vec4 out_color;
in vec3 texcoords;

layout (binding = 0) uniform samplerCube skybox;

void main() {
    out_color = texture(skybox, texcoords);
}
)";

/**
 * Fullscreen quad vertex shader
 * This shader works with the rcube::common::fullScreenQuadMesh() mesh and
 * outputs a fullscreen quad and the texture coordinate at each fragment (v_texcoord)
 * The fragment shader must be defined by the user.
 */
const static std::string FULLSCREEN_QUAD_VERTEX_SHADER = R"(
#version 450 core

layout (location = 0) in vec3 position;
layout (location = 2) in vec2 uv;
out vec2 v_texcoord;

void main() {
    gl_Position = vec4(position, 1);
    v_texcoord = uv;
}
)";

const static std::string FULLSCREEN_QUAD_TEXTURE_FRAGMENT_SHADER = R"(
#version 450 core

in vec2 v_texcoord;
out vec4 out_color;
layout (binding=0) uniform sampler2D tex;

void main() {
   out_color = texture(tex, v_texcoord);
}
)";

/**
 * Create a shader for rendering fullscreen quad with a user-defined fragment shader
 * The shader works with a quad mesh generated using rcube::common::fullscreenQuadMesh() and
 * outputs a texture coordinate named "v_texcoord" that should be used in the fragment shader.
 */
std::shared_ptr<ShaderProgram> fullScreenQuadShader(const std::string &fragment_shader);

/**
 * Create a shader for rendering a skybox
 * The shader works with a skybox mesh generated using rcube::common::skyboxMesh() and
 * expects a cubemap to be bound to texture unit 2.
 */
std::shared_ptr<ShaderProgram> skyboxShader();

} // namespace common
} // namespace rcube