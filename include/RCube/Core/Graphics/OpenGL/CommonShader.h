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
#version 450

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
#version 450

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
#version 450

layout (location = 0) in vec3 position;
layout (location = 2) in vec2 uv;
out vec2 v_texcoord;

void main() {
    gl_Position = vec4(position, 1);
    v_texcoord = uv;
}
)";

const static std::string FULLSCREEN_QUAD_TEXTURE_FRAGMENT_SHADER = R"(
#version 450

in vec2 v_texcoord;
out vec4 out_color;
layout (binding=0) uniform sampler2D tex;

void main() {
   out_color = texture(tex, v_texcoord);
}
)";


/**
 * Shadowmap vertex shader
 */
const static std::string SHADOWMAP_VERTEX_SHADER = R"(
#version 450

layout (location = 0) in vec3 position;

uniform mat4 light_matrix;
uniform mat4 model_matrix;
invariant gl_Position;

void main()
{
    gl_Position = light_matrix * model_matrix * vec4(position, 1.0);
}
)";

/**
 * Shadowmap fragment shader
 */
const static std::string SHADOWMAP_FRAGMENT_SHADER = R"(
#version 450

void main() {
    // gl_FragDepth = gl_FragCoord.z;
}
)";


/**
 * Unique color for entity: fragment shader
 */
const static std::string UNIQUECOLOR_FRAGMENT_SHADER = R"(
#version 450

uniform int id;
layout (location = 0) out ivec3 frag_out;

void main() {
    frag_out = ivec3(id, gl_PrimitiveID, 0);
}
)";

/**
 * Unique color for entity: vertex shader
 */
const static std::string UNIQUECOLOR_VERTEX_SHADER = R"(
#version 450

layout (location = 0) in vec3 position;

layout (std140, binding=0) uniform Camera {
    mat4 view_matrix;
    mat4 projection_matrix;
    mat4 viewport_matrix;
    vec3 eye_pos;
};

uniform mat4 model_matrix;
invariant gl_Position;

void main()
{
    gl_Position = projection_matrix * view_matrix * model_matrix * vec4(position, 1.0);
}
)";


/**
 * Depth pass: fragment shader
 */
const static std::string DEPTH_FRAGMENT_SHADER = R"(
#version 450

void main()
{
}
)";


/**
 * Depth pass: vertex shader
 */
const static std::string DEPTH_VERTEX_SHADER = R"(
#version 450

layout (location = 0) in vec3 position;

layout (std140, binding=0) uniform Camera {
    mat4 view_matrix;
    mat4 projection_matrix;
    mat4 viewport_matrix;
    vec3 eye_pos;
};

uniform mat4 model_matrix;
invariant gl_Position;

void main()
{
    vec4 world_position = model_matrix * vec4(position, 1.0);
    gl_Position = projection_matrix * view_matrix * world_position;
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

/**
 * Create a shader for rendering a shadow map into a depth buffer
 */
std::shared_ptr<ShaderProgram> shadowMapShader();

/**
 * Create a shader for rendering each entity with a unique ID
 */
std::shared_ptr<ShaderProgram> uniqueColorShader();

/**
 * Create a shader for depth pre-pass
 */
std::shared_ptr<ShaderProgram> depthShader();


} // namespace common
} // namespace rcube