#include "RCube/Core/Graphics/Materials/BlinnPhongMaterial.h"
#include "stb_image/stb_image.h"

namespace rcube
{

const std::string vert_str =
    R"(
#version 420
layout (location = 0) in vec3 vertex;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texcoord;
layout (location = 3) in vec3 color;
layout (location = 4) in vec3 tangent;

layout (std140, binding=0) uniform Camera {
    mat4 view_matrix;
    mat4 projection_matrix;
    mat4 viewport_matrix;
    vec3 eye_pos;
};

uniform mat4 model_matrix;
uniform mat3 normal_matrix;

out vec3 v_vertex;
out vec3 v_normal;
out vec3 v_color;
out vec2 v_texture;
out mat3 v_tbn;

void main() {
    vec4 world_vertex = model_matrix * vec4(vertex, 1.0);
    gl_Position = projection_matrix * view_matrix * world_vertex;
    v_vertex = world_vertex.xyz;
    v_normal = normalize(normal_matrix * normal);
    v_color = color;
    v_texture = texcoord;
    vec3 T = normalize(vec3(model_matrix * vec4(tangent, 0.0)));
    vec3 B = cross(v_normal, T);
    v_tbn = mat3(T, B, v_normal);
}
)";

const std::string geom_str =
    R"(
#version 420
layout (triangles) in;
layout (triangle_strip, max_vertices=3) out;

layout (std140, binding=0) uniform Camera {
    mat4 view_matrix;
    mat4 projection_matrix;
    mat4 viewport_matrix;
    vec3 eye_pos;
};

in vec3 v_vertex[];
out vec3 g_vertex;
in vec3 v_normal[];
out vec3 g_normal;
in vec2 v_texture[];
out vec2 g_texture;
in vec3 v_color[];
out vec3 g_color;
in mat3 v_tbn[];
out mat3 g_tbn;

noperspective out vec3 dist;

void main() {
    // Transform each vertex into viewport space
    vec3 p0 = vec3(viewport_matrix * (gl_in[0].gl_Position / gl_in[0].gl_Position.w));
    vec3 p1 = vec3(viewport_matrix * (gl_in[1].gl_Position / gl_in[1].gl_Position.w));
    vec3 p2 = vec3(viewport_matrix * (gl_in[2].gl_Position / gl_in[2].gl_Position.w));

    float a = length(p1 - p2);
    float b = length(p2 - p0);
    float c = length(p0 - p1);

    // Interior angles
    float alpha = acos((b*b + c*c - a*a) / (2.0 * b * c));
    float beta = acos((a*a + c*c - b*b) / (2.0 * a * c));

    // Distance from vertex to opposite side using law of cosines
    float ha = c * sin(beta);
    float hb = c * sin(alpha);
    float hc = b * sin(alpha);

    // Emit vertex 1
    dist = vec3(ha, 0, 0);
    g_vertex = v_vertex[0];
    g_normal = v_normal[0];
    g_texture = v_texture[0];
    g_color = v_color[0];
    g_tbn = v_tbn[0];
    gl_Position = gl_in[0].gl_Position;
    EmitVertex();

    // Emit vertex 2
    dist = vec3(0, hb, 0);
    g_vertex = v_vertex[1];
    g_normal = v_normal[1];
    g_texture = v_texture[1];
    g_color = v_color[1];
    g_tbn = v_tbn[1];
    gl_Position = gl_in[1].gl_Position;
    EmitVertex();

    // Emit vertex 3
    dist = vec3(0, 0, hc);
    g_vertex = v_vertex[2];
    g_normal = v_normal[2];
    g_texture = v_texture[2];
    g_color = v_color[2];
    g_tbn = v_tbn[2];
    gl_Position = gl_in[2].gl_Position;
    EmitVertex();
    EndPrimitive();
}
)";

const std::string frag_str =
    R"(
#version 420

#define MAX_LIGHTS 99

// Interpolated input from geometry shader
in vec3 g_vertex;
in vec3 g_normal;
in vec2 g_texture;
in vec3 g_color;
noperspective in vec3 dist;

// Fragment shader output
out vec4 out_color;

// --------------------------------
// Camera Uniform Block
// --------------------------------
layout (std140, binding=0) uniform Camera {
    mat4 view_matrix;
    mat4 projection_matrix;
    mat4 viewport_matrix;
    vec3 eye_pos;
};
in mat3 g_tbn;

// --------------------------------
// Light Uniform Block
// --------------------------------
struct Light {
    vec4 position;
    vec4 direction_radius;
    vec4 color_coneangle;
};

layout (std140, binding=2) uniform Lights {
    Light lights[MAX_LIGHTS];
    int num_lights;
};

// --------------------------------
// Material properties
// --------------------------------
uniform vec3 diffuse;
uniform vec3 specular;
uniform float shininess;
uniform float reflectivity;

layout(binding=0) uniform sampler2D diffuse_tex;
layout(binding=1) uniform sampler2D specular_tex;
layout(binding=2) uniform sampler2D normal_tex;
layout(binding=3) uniform samplerCube env_map;

uniform bool show_wireframe;
uniform bool show_backface;
uniform bool use_diffuse_texture, use_specular_texture, use_normal_texture, use_environment_map;
uniform int blend_environment_map;

uniform vec3 wireframe_color;
uniform float wireframe_thickness;

// Returns the attenuation factor that is multiplied with the light's color
float attenuation(float dist, float radius) {
    // float att = clamp(1.0 - dist * dist / (radius * radius), 0.0, 1.0);
    // return att * att;
    return 1.0 / (1.0 + (dist * dist) / (radius * radius));
}

bool close(float a, float b) {
    return abs(a - b) < 0.00001;
}

void main() {
    vec3 result = vec3(0.0);
    // Diffuse component
    vec3 material_diffuse = use_diffuse_texture ? texture(diffuse_tex, g_texture).rgb * g_color : diffuse * g_color;
    // Specular component
    float specular_tex_val = use_specular_texture ? texture(specular_tex, g_texture).r : 1.0;
    // Surface normal
    vec3 N = use_normal_texture ? g_tbn * (texture(normal_tex, g_texture).rgb * 2.0 - 1.0) : g_normal;
    N = normalize(N);
    // Surface to eye
    vec3 V = normalize(vec3(eye_pos - g_vertex)); // Surface to eye

    for (int i = 0; i < min(num_lights, MAX_LIGHTS); ++i)
    {
        vec3 L;          // Surface to light
        float att = 1.0; // Light attenuation

        if (close(lights[i].position.w, 0.0)) { // is directional?
            L = lights[i].position.xyz;
        }
        else {
            L = lights[i].position.xyz - g_vertex;
            // att = attenuation(length(L), lights[i].radius);
            att = attenuation(length(L), lights[i].direction_radius.w);
        }
        L = normalize(L);

        // Accumulate output color using each light source
        vec3 H = normalize(L + V);  // Halfway vector
        float LdotN = dot(L, N);
        float diff_contrib = show_backface ? abs(LdotN) : max(LdotN, 0.0);
        float spec_contrib = 0.0;
        if (LdotN > 0.0) {
            spec_contrib = specular_tex_val * pow(max(0, dot(N, H)), shininess);
        }
        vec3 light_color = att * lights[i].color_coneangle.xyz;
        result += light_color * (diff_contrib * material_diffuse + spec_contrib * specular);
    }

    // Wireframe
    if (show_wireframe) {
        // Find the smallest distance
        float d = min(dist.x, dist.y);
        d = min(d, dist.z);
        float mix_val = smoothstep(wireframe_thickness - 1.0, wireframe_thickness + 1.0, d);
        result = mix(wireframe_color, result, mix_val);
    }
    // Environment map
    if (use_environment_map) {
        vec3 I = normalize(g_vertex - eye_pos);
        vec3 R = reflect(I, normalize(g_normal));
        //R = vec3(inverse(view_matrix) * vec4(R, 0.0));
        vec3 em = texture(env_map, R).rgb;
        if (blend_environment_map == 0) {
            result *= em;
        }
        else if (blend_environment_map == 1) {
            result += em;
        }
        else if (blend_environment_map == 2) {
            result = mix(em, result, reflectivity);
        }
    }

    // Output
    out_color = vec4(result, 1.0);
}
)";

const static VertexShader BlinnPhongVertexShader = {
    /*attributes: */
    {ShaderAttributeDesc("vertex", GLDataType::Vec3f),
     ShaderAttributeDesc("normal", GLDataType::Vec3f),
     ShaderAttributeDesc("color", GLDataType::Vec3f),
     ShaderAttributeDesc("texcoord", GLDataType::Vec2f),
     ShaderAttributeDesc("tangent", GLDataType::Vec3f)},
    /*uniforms: */
    {{"model_matrix", GLDataType::Mat4f}, {"normal_matrix", GLDataType::Mat3f},
     /*{"eye_pos", GLDataType::Vec3f}*/},
    vert_str};

const static GeometryShader BlinnPhongGeometryShader = {
    /*attributes: */
    {},
    /*uniforms: */
    {},
    geom_str};

const static FragmentShader BlinnPhongFragmentShader = {
    /*uniforms: */
    {{"diffuse", GLDataType::Color3f},
     {"specular", GLDataType::Color3f},
     {"shininess", GLDataType::Float},
     {"reflectivity", GLDataType::Float},
     {"show_wireframe", GLDataType::Bool},
     {"show_backface", GLDataType::Bool},
     {"wireframe_color", GLDataType::Color3f},
     {"wireframe_thickness", GLDataType::Float},
     {"use_diffuse_texture", GLDataType::Bool},
     {"use_specular_texture", GLDataType::Bool},
     {"use_normal_texture", GLDataType::Bool},
     {"use_environment_map", GLDataType::Bool},
     {"blend_environment_map", GLDataType::Int}},
    /*textures: */
    {ShaderTextureDesc{"diffuse_tex", 2}, ShaderTextureDesc{"specular_tex", 2},
     ShaderTextureDesc{"normal_tex", 2}},
    /*cubemaps: */
    {ShaderCubemapDesc{"env_map"}},
    "out_color",
    frag_str};

std::shared_ptr<ShaderProgram> makeBlinnPhongMaterial(glm::vec3 diffuse_color,
                                                      glm::vec3 specular_color, float shininess,
                                                      bool wireframe)
{
    auto prog = ShaderProgram::create(BlinnPhongVertexShader, BlinnPhongGeometryShader,
                                      BlinnPhongFragmentShader, true);
    prog->renderPriority() = RenderPriority::Opaque;
    prog->uniform("diffuse").set(diffuse_color);
    prog->uniform("specular").set(specular_color);
    prog->uniform("shininess").set(shininess);
    prog->uniform("show_wireframe").set(wireframe);
    prog->uniform("wireframe_thickness").set(1.f);
    prog->uniform("blend_environment_map").set(1);
    prog->renderState().depth_test = true;
    prog->renderState().depth_write = true;
    prog->renderState().blending = false;
    prog->renderState().culling = false;
    return prog;
}

} // namespace rcube
