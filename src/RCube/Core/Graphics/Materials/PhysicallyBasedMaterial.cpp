#include "RCube/Core/Graphics/Materials/PhysicallyBasedMaterial.h"

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

const static VertexShader PBRVertexShader = {{ShaderAttributeDesc("vertex", GLDataType::Vec3f),
                                              ShaderAttributeDesc("normal", GLDataType::Vec3f),
                                              ShaderAttributeDesc("texcoord", GLDataType::Vec2f),
                                              ShaderAttributeDesc("normal", GLDataType::Vec3f),
                                              ShaderAttributeDesc("tangent", GLDataType::Vec3f)},
                                             {ShaderUniformDesc{"model_matrix", GLDataType::Mat4f},
                                              ShaderUniformDesc{"normal_matrix", GLDataType::Mat3f}},
                                             vert_str}; // namespace rcube

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

const static GeometryShader PBRGeometryShader = {{}, {}, geom_str};

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

layout (std140, binding=0) uniform Camera {
    mat4 view_matrix;
    mat4 projection_matrix;
    mat4 viewport_matrix;
    vec3 eye_pos;
};
in mat3 g_tbn;

// --------------------------------
// Light uniforms
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
struct Material {
    vec3 albedo;
    float roughness;
    float metalness;
};
uniform Material material;

layout (binding = 0) uniform sampler2D albedo_tex;
layout (binding = 1) uniform sampler2D roughness_tex;
layout (binding = 2) uniform sampler2D metalness_tex;
layout (binding = 3) uniform sampler2D normal_tex;
layout (binding = 4) uniform samplerCube irradiance_map;

uniform bool show_wireframe;
uniform bool use_albedo_texture, use_roughness_texture, use_metalness_texture, use_normal_texture, use_irradiance_map;

struct Line {
    vec3 color;
    float thickness;
};
uniform Line line_props;

// Returns the attenuation factor that is multiplied with the light's color
float falloff(float dist, float radius) {
    float denom = (dist * dist) / (radius * radius);
    return 1.0 / denom;
}

float falloffEpic(float dist, float radius) {
    float tmp = 1.0 - pow(dist / radius, 4);
    tmp = pow(clamp(tmp, 0, 1), 2);
    return tmp / (dist * dist + 1);
}

const float PI = 3.14159265359;

bool close(float a, float b) {
    return abs(a - b) < 0.00001;
}

float DGgx(float HdotN, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float HdotN2 = HdotN * HdotN;

    float nom   = a2;
    float denom = (HdotN2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / max(denom, 0.001);
}

float GSchlickGgx(float NdotV, float roughness) {
    float r = 1.0 + roughness;
    float k = (r * r) / 8.0;

    float numerator = NdotV;
    float denominator = NdotV * (1.0 - k) + k;

    return numerator / max(denominator, 0.001);
}

float GSmith(float NdotV, float LdotN, float roughness) {
    float ggx1 = GSchlickGgx(LdotN, roughness);
    float ggx2 = GSchlickGgx(NdotV, roughness);
    return ggx1 * ggx2;
}

vec3 FSchlick(float cos_grazing_angle, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cos_grazing_angle, 0.0, 1.0), 5.0);
}

vec3 diffuseLambertian(vec3 albedo) {
    return albedo / PI;
}

void main() {
    vec3 result = vec3(0.0);
    // Albedo
    vec3 albedo = use_albedo_texture ? texture(albedo_tex, g_texture).rgb * g_color : material.albedo * g_color;
    // Roughness
    float roughness = use_roughness_texture ? texture(roughness_tex, g_texture).r : material.roughness;
    roughness = clamp(roughness, 0.04, 1.0);
    // metalness
    float metalness = use_metalness_texture ? texture(metalness_tex, g_texture).r : material.metalness;
    metalness = clamp(metalness, 0.0, 1.0);
    // Surface normal
    vec3 N = use_normal_texture ? g_tbn * (texture(normal_tex, g_texture).rgb * 2.0 - 1.0) : g_normal;
    N = normalize(N);
    // Surface to eye
    vec3 V = normalize(vec3(eye_pos - g_vertex));
    float NdotV = max(dot(N, V), 0);
    // Specular color
    vec3 specular_color = vec3(0.04);
    specular_color = mix(specular_color, albedo, material.metalness);

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
            att = falloff(length(L), lights[i].direction_radius.w);
        }
        L = normalize(L);
        // Radiance
        vec3 radiance = att * lights[i].color_coneangle.xyz;

        // Useful values to precompute
        vec3 H = normalize(L + V);  // Halfway vector
        float LdotN = max(dot(L, N), 0);
        float HdotV = max(dot(H, V), 0);
        float HdotN = max(dot(H, N), 0);

        // Cook-Torrance BRDF
        float D = DGgx(HdotN, roughness);
        float G = GSmith(NdotV, LdotN, roughness);
        vec3 F = FSchlick(HdotV, specular_color);
        vec3 specular_contrib = (D * G * F) / max(4.0 * NdotV * LdotN, 0.001);

        // Lambertian BRDF
        vec3 diffuse_contrib = vec3(1.0) - F;
        diffuse_contrib *= 1.0 - metalness; // Metallic materials have ~0 diffuse contribution
        diffuse_contrib *= diffuseLambertian(albedo);
        result += (diffuse_contrib + specular_contrib) * radiance * LdotN;
    }
    // Indirect image-based lighting
    if (use_irradiance_map) {
        vec3 irradiance = texture(irradiance_map, N).rgb;
        vec3 kS = FSchlick(NdotV, specular_color);
        vec3 kD = 1.0 - kS;
        kD *= 1.0 - metalness;
        vec3 diffuse = irradiance * albedo;
        vec3 ambient = (kD * diffuse) * 1.0;
        //vec3 ambient = vec3(0.03) * albedo * 1.0;
        result += ambient;
    }

    // Wireframe
    if (show_wireframe) {
        // Find the smallest distance
        float d = min(dist.x, dist.y);
        d = min(d, dist.z);
        float mix_val = smoothstep(line_props.thickness - 1.0, line_props.thickness + 1.0, d);
        result = mix(line_props.color, result, mix_val);
    }

    out_color = vec4(result, 1.0);
}
)";

const static FragmentShader PBRFragmentShader = {
    {ShaderUniformDesc{"material.albedo", GLDataType::Vec3f},
     ShaderUniformDesc{"material.roughness", GLDataType::Float},
     ShaderUniformDesc{"material.metalness", GLDataType::Float},
     ShaderUniformDesc{"show_wireframe", GLDataType::Bool},
     ShaderUniformDesc{"use_albedo_texture", GLDataType::Bool},
     ShaderUniformDesc{"use_roughness_texture", GLDataType::Bool},
     ShaderUniformDesc{"use_metalness_texture", GLDataType::Bool},
     ShaderUniformDesc{"use_normal_texture", GLDataType::Bool},
     ShaderUniformDesc{"use_irradiance_map", GLDataType::Bool},
     ShaderUniformDesc{"line_props.color", GLDataType::Vec3f},
     ShaderUniformDesc{"line_props.thickness", GLDataType::Float}},
    {ShaderTextureDesc{"albedo_tex", 2}, ShaderTextureDesc{"roughness_tex", 2},
     ShaderTextureDesc{"metalness_tex", 2}, ShaderTextureDesc{"normal_tex", 2}},
    {ShaderCubemapDesc{"irradiance_map"}},
    "out_color",
    frag_str};

std::shared_ptr<ShaderProgram> makePhysicallyBasedMaterial(glm::vec3 albedo, float roughness,
                                                           float metalness, bool wireframe)
{
    auto prog = ShaderProgram::create(PBRVertexShader, PBRGeometryShader, PBRFragmentShader, true);
    prog->uniform("material.albedo").set(albedo);
    prog->uniform("material.roughness").set(roughness);
    prog->uniform("material.metalness").set(metalness);
    prog->uniform("show_wireframe").set(wireframe);
    prog->uniform("use_albedo_texture").set(false);
    prog->uniform("use_roughness_texture").set(false);
    prog->uniform("use_metalness_texture").set(false);
    prog->uniform("use_normal_texture").set(false);

    prog->renderState().depth_test = true;
    prog->renderState().depth_write = true;
    prog->renderState().blending = false;
    prog->renderState().culling = false;

    prog->renderPriority() = RenderPriority::Opaque;
    return prog;
}

} // namespace rcube
