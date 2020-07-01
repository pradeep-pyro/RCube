#include "RCube/Core/Graphics/Materials/PhysicallyBasedMaterial.h"
#include "RCube/Core/Graphics/ImageBasedLighting/IBLDiffuse.h"
#include "RCube/Core/Graphics/ImageBasedLighting/IBLSpecularSplitSum.h"
#include "imgui.h"

namespace rcube
{

const std::string PBRVertexShader =
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

const static std::string PBRGeometryShader =
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

const static std::string PBRFragmentShader =
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
    float metallic;
};
uniform Material material;

layout(binding=0) uniform sampler2D albedo_tex;
layout(binding=1) uniform sampler2D roughness_tex;
layout(binding=2) uniform sampler2D metallic_tex;
layout(binding=3) uniform sampler2D normal_tex;

layout(binding=4) uniform sampler2D brdf_lut;
layout(binding=5) uniform samplerCube prefilter_map;
layout(binding=6) uniform samplerCube irradiance_map;

uniform bool show_wireframe;
uniform bool use_albedo_texture, use_roughness_texture, use_metallic_texture, use_normal_texture;
uniform bool use_image_based_lighting;

struct Wireframe {
    bool show;
    vec3 color;
    float thickness;
};
uniform Wireframe wireframe;

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

float DGgx(vec3 N, vec3 H, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float HdotN = max(dot(H, N), 0.0);
    float HdotN2 = HdotN * HdotN;

    float nom   = a2;
    float denom = (HdotN2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}

float GSchlickGgx(float NdotV, float roughness) {
    float r = 1.0 + roughness;
    float k = (r * r) / 8.0;

    float numerator = NdotV;
    float denominator = NdotV * (1.0 - k) + k;

    return numerator / denominator;
}

float GSmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float LdotN = max(dot(L, N), 0.0);
    float ggx1 = GSchlickGgx(LdotN, roughness);
    float ggx2 = GSchlickGgx(NdotV, roughness);
    return ggx1 * ggx2;
}

vec3 FSchlick(float cos_grazing_angle, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cos_grazing_angle, 0.0, 1.0), 5.0);
}

vec3 FSchlickRoughness(float cos_grazing_angle, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - cos_grazing_angle, 5.0);
}

vec3 diffuseLambertian(vec3 albedo) {
    return albedo / PI;
}

void main() {
    // Albedo: convert sRGB albedo texture to linear by pow(x, 2.2)
    vec3 albedo = use_albedo_texture ? texture(albedo_tex, g_texture).rgb * g_color : material.albedo * g_color;
    //vec3 albedo = use_albedo_texture ? pow(texture(albedo_tex, g_texture).rgb, vec3(2.2)) * g_color : material.albedo * g_color;
    albedo = pow(albedo, vec3(2.2));
    // Roughness
    float roughness = use_roughness_texture ? texture(roughness_tex, g_texture).r : material.roughness;
    roughness = clamp(roughness, 0.04, 1.0);
    // metallic
    float metallic = use_metallic_texture ? texture(metallic_tex, g_texture).r : material.metallic;
    metallic = clamp(metallic, 0.0, 1.0);
    // Surface normal
    vec3 N = use_normal_texture ? g_tbn * (texture(normal_tex, g_texture).rgb * 2.0 - 1.0) : g_normal;
    N = normalize(N);
    // Surface to eye
    vec3 V = normalize(vec3(eye_pos - g_vertex));
    float NdotV = max(dot(N, V), 0);
    // Specular color
    vec3 specular_color = vec3(0.04);
    specular_color = mix(specular_color, albedo, metallic);

    vec3 direct_result = vec3(0.0);
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

        // Cook-Torrance specular BRDF
        float D = DGgx(N, H, roughness);
        float G = GSmith(N, V, L, roughness);
        vec3 F = FSchlick(max(dot(H, V), 0.0), specular_color);
        vec3 numer = D * G * F;
        float denom = 4 * max(dot(N, V), 0.0) * max(dot(L, N), 0.0) + 0.001;
        vec3 specular = numer / denom;
        vec3 kS = F;

        // Lambertian BRDF
        vec3 diffuse = diffuseLambertian(albedo);
        vec3 kD = vec3(1.0) - kS;
        kD *= 1.0 - metallic; // Metallic materials have ~0 diffuse contribution
        direct_result += (kD * diffuse + specular) * radiance * dot(L, N);
    }

    // Indirect image-based lighting for ambient term
    vec3 ambient = vec3(0.03) * albedo; // default ambient color for non-IBL setting
    if (use_image_based_lighting) {
        vec3 F = FSchlickRoughness(max(dot(N, V), 0.0), specular_color, roughness);
        vec3 kS = F;
        vec3 kD = 1.0 - kS;
        kD *= 1.0 - metallic;

        vec3 irradiance = texture(irradiance_map, N).rgb;
        vec3 diffuse = irradiance * albedo;
        
        const float MAX_REFLECTION_LOD = 4.0;
        vec3 R = reflect(-V, N);
        vec3 prefiltered_color = textureLod(prefilter_map, R,  roughness * MAX_REFLECTION_LOD).rgb;
        vec2 brdf  = texture(brdf_lut, vec2(max(dot(N, V), 0.0), roughness)).rg;
        vec3 specular = prefiltered_color * (F * brdf.x + brdf.y);

        ambient = (kD * diffuse + specular) * 1.0;
    }
    vec3 result = direct_result + ambient;

    // Wireframe
    if (wireframe.show) {
        // Find the smallest distance
        float d = min(dist.x, dist.y);
        d = min(d, dist.z);
        float mix_val = smoothstep(wireframe.thickness - 1.0, wireframe.thickness + 1.0, d);
        result = mix(wireframe.color, result, mix_val);
    }

    // Tone mapping using Reinhard operator
    result = result / (result + vec3(1.0));

    // Output
    out_color = vec4(result, 1.0);
    
}
)";

PhysicallyBasedMaterial::PhysicallyBasedMaterial()
{
    // TODO(pradeep): Shader programs must be re-used among different material objects
    shader_ = ShaderProgram::create(PBRVertexShader, PBRGeometryShader, PBRFragmentShader, true);
}

void PhysicallyBasedMaterial::setUniforms()
{
    const bool use_ibl =
        irradiance_map != nullptr && prefilter_map != nullptr && brdf_map != nullptr;
    // Set uniforms
    shader_->uniform("material.albedo").set(albedo);
    shader_->uniform("material.roughness").set(roughness);
    shader_->uniform("material.metallic").set(metallic);
    shader_->uniform("wireframe.show").set(wireframe);
    shader_->uniform("wireframe.thickness").set(wireframe_thickness);
    shader_->uniform("wireframe.color").set(wireframe_color);
    shader_->uniform("use_albedo_texture").set(albedo_texture != nullptr);
    shader_->uniform("use_roughness_texture").set(roughness_texture != nullptr);
    shader_->uniform("use_metallic_texture").set(metallic_texture != nullptr);
    shader_->uniform("use_normal_texture").set(normal_texture != nullptr);
    shader_->uniform("use_image_based_lighting").set(use_ibl);

    // Bind textures at units
    // TODO(pradeep): these sampler locations can be cached
    if (albedo_texture != nullptr)
    {
        int unit;
        shader_->uniform("albedo_tex").get(unit);
        albedo_texture->use(unit);
    }
    if (metallic_texture != nullptr)
    {
        int unit;
        shader_->uniform("metallic_tex").get(unit);
        metallic_texture->use(unit);
    }
    if (normal_texture != nullptr)
    {
        int unit;
        shader_->uniform("normal_tex").get(unit);
        normal_texture->use(unit);
    }
    if (use_ibl)
    {
        int irr_unit, pre_unit, brdf_unit;
        shader_->uniform("irradiance_map").get(irr_unit);
        shader_->uniform("prefilter_map").get(pre_unit);
        shader_->uniform("brdf_lut").get(brdf_unit);
        irradiance_map->use(irr_unit);
        prefilter_map->use(pre_unit);
        brdf_map->use(brdf_unit);
    }
}

void PhysicallyBasedMaterial::drawGUI()
{
    ImGui::Text("Physically-based Material");
    ImGui::ColorEdit3("Albedo", glm::value_ptr(albedo));
    ImGui::SliderFloat("Roughness", &roughness, 0.04f, 1.f);
    ImGui::SliderFloat("Metallic", &metallic, 0.f, 1.f);
    ImGui::Text("Wireframe");
    ImGui::Checkbox("Show", &wireframe);
    ImGui::InputFloat("Thickness", &wireframe_thickness);
    ImGui::ColorEdit3("Color", glm::value_ptr(wireframe_color));
}

void PhysicallyBasedMaterial::setIBLMaps(std::shared_ptr<TextureCubemap> irradiance,
                                         std::shared_ptr<TextureCubemap> prefilter,
                                         std::shared_ptr<Texture2D> brdf)
{
    irradiance_map = irradiance;
    prefilter_map = prefilter;
    brdf_map = brdf;
}

void PhysicallyBasedMaterial::createIBLMaps(std::shared_ptr<TextureCubemap> environment_map)
{
    IBLDiffuse diff;
    irradiance_map = diff.irradiance(environment_map);
    IBLSpecularSplitSum spec;
    prefilter_map = spec.prefilter(environment_map);
    brdf_map = spec.integrateBRDF();
}

const RenderSettings PhysicallyBasedMaterial::renderState() const
{
    RenderSettings state;
    state.depth_test = true;
    state.depth_write = true;
    state.blending = false;
    state.culling = false;
    return state;
}

} // namespace rcube
