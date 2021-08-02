#include "RCube/Materials/StandardMaterial.h"
#include "RCube/Core/Graphics/ShaderManager.h"
#include "imgui.h"
#include <string>

namespace rcube
{

const std::string StandardVertexShader =
    R"(
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 uv;
layout (location = 3) in vec3 color;
layout (location = 4) in vec3 tangent;
layout (location = 5) in float wire;

layout (std140, binding=0) uniform Camera {
    mat4 view_matrix;
    mat4 projection_matrix;
    mat4 viewport_matrix;
    vec3 eye_pos;
};

out vec3 vert_position;
out vec2 vert_uv;
out vec3 vert_color;
out vec3 vert_normal;
out mat3 vert_tbn;
flat out float vert_wire;

uniform mat4 model_matrix;
uniform mat3 normal_matrix;

void main()
{
    vec4 world_pos = model_matrix * vec4(position, 1.0);
    vert_position = world_pos.xyz;
    vert_uv = uv;
    vert_color = pow(color, vec3(2.2));
    vert_normal = normal_matrix * normal;
    gl_Position = projection_matrix * view_matrix * world_pos;
    // Tangent basis
    vec3 T = normalize(vec3(model_matrix * vec4(tangent, 0.0)));
    vec3 B = cross(normal, T);
    vert_tbn = mat3(T, B, normal);
    vert_wire = wire;
}
)";

const static std::string StandardGeometryShader =
    R"(
layout (triangles) in;
layout (triangle_strip, max_vertices=3) out;

layout (std140, binding=0) uniform Camera {
    mat4 view_matrix;
    mat4 projection_matrix;
    mat4 viewport_matrix;
    vec3 eye_pos;
};

in vec3 vert_position[];
out vec3 geom_position;
in vec3 vert_normal[];
out vec3 geom_normal;
in vec2 vert_uv[];
out vec2 geom_uv;
in vec3 vert_color[];
out vec3 geom_color;
in mat3 vert_tbn[];
out mat3 geom_tbn;
flat in float vert_wire[];
out float geom_wire;

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
    geom_position = vert_position[0];
    geom_normal = vert_normal[0];
    geom_uv = vert_uv[0];
    geom_color = vert_color[0];
    geom_tbn = vert_tbn[0];
    geom_wire = vert_wire[0];
    gl_Position = gl_in[0].gl_Position;
    EmitVertex();

    // Emit vertex 2
    dist = vec3(0, hb, 0);
    geom_position = vert_position[1];
    geom_normal = vert_normal[1];
    geom_uv = vert_uv[1];
    geom_color = vert_color[1];
    geom_tbn = vert_tbn[1];
    geom_wire = vert_wire[1];
    gl_Position = gl_in[1].gl_Position;
    EmitVertex();

    // Emit vertex 3
    dist = vec3(0, 0, hc);
    geom_position = vert_position[2];
    geom_normal = vert_normal[2];
    geom_uv = vert_uv[2];
    geom_color = vert_color[2];
    geom_tbn = vert_tbn[2];
    geom_wire = vert_wire[2];
    gl_Position = gl_in[2].gl_Position;
    EmitVertex();
    EndPrimitive();
}
)";

const std::string StandardFragmentShader =
    R"(
in vec3 geom_position;
in vec3 geom_normal;
in vec2 geom_uv;
in vec3 geom_color;
in mat3 geom_tbn;
in float geom_wire;
noperspective in vec3 dist;

#define RCUBE_MAX_DIRLIGHTS 5
#define RCUBE_MAX_POINTLIGHTS 50

#if RCUBE_RENDERPASS == 0
out vec4 out_color;
#elif RCUBE_RENDERPASS == 1
layout (location = 0) out vec4 accum;
layout (location = 1) out float reveal;
#endif

uniform vec3 albedo;
uniform float roughness;
uniform float metallic;

uniform bool use_albedo_texture;
uniform bool use_roughness_texture;
uniform bool use_metallic_texture;
uniform bool use_normal_texture;

layout(binding=0) uniform sampler2D albedo_tex;
layout(binding=1) uniform sampler2D roughness_tex;
layout(binding=2) uniform sampler2D metallic_tex;
layout(binding=3) uniform sampler2D normal_tex;
layout(binding=4) uniform sampler2D brdf_lut;
layout(binding=5) uniform samplerCube prefilter_map;
layout(binding=6) uniform samplerCube irradiance_map;
uniform bool use_image_based_lighting;

struct Wireframe {
    bool show;
    vec3 color;
    float thickness;
};
uniform Wireframe wireframe;
uniform bool show_wireframe;
uniform float opacity;

const vec3 PINK = pow(vec3(255.0 / 255.0, 20.0 / 255.0, 147.0 / 255.0), vec3(2.2));
const vec3 PURPLE = pow(vec3(138.0 / 255.0, 43.0 / 255.0, 226.0 / 255.0), vec3(2.2));

layout (std140, binding=0) uniform Camera {
    mat4 view_matrix;
    mat4 projection_matrix;
    mat4 viewport_matrix;
    vec3 eye_pos;
};

struct DirectionalLight
{
    vec3 direction;
    float cast_shadow;
    vec3 color;
    float intensity;
    mat4 view_proj;
};

layout (std140, binding=1) uniform DirectionalLights {
    DirectionalLight dirlights[RCUBE_MAX_DIRLIGHTS];
    int num_dirlights;
};

struct PointLight
{
    vec3 position;
    float cast_shadow;
    vec3 color;
    float radius;
    float intensity;
    float padding1_;
    float padding2_;
    float padding3_;
};

layout (std140, binding=2) uniform PointLights {
    PointLight pointlights[RCUBE_MAX_POINTLIGHTS];
    int num_pointlights;
};

struct PerLightDotProducts
{
    float LdotN, HdotV, HdotN;
};

// Reference:
// https://digitalrune.github.io/DigitalRune-Documentation/html/fa431d48-b457-4c70-a590-d44b0840ab1e.htm
const mat4 bayerMatrix = mat4(
    vec4(1.0 / 17.0,  9.0 / 17.0,  3.0 / 17.0, 11.0 / 17.0),
    vec4(13.0 / 17.0,  5.0 / 17.0, 15.0 / 17.0,  7.0 / 17.0),
    vec4(4.0 / 17.0, 12.0 / 17.0,  2.0 / 17.0, 10.0 / 17.0),
    vec4(16.0 / 17.0,  8.0 / 17.0, 14.0 / 17.0,  6.0 / 17.0)
);

// Returns the attenuation factor that is multiplied with the light's color
float falloff(float dist, float radius) {
    float denom = (dist * dist) / (radius * radius);
    return 1.0 / denom;
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

    return nom / denom;
}

float GSchlickGgx(float NdotV, float roughness) {
    float r = 1.0 + roughness;
    float k = (r * r) / 8.0;

    float numerator = NdotV;
    float denominator = NdotV * (1.0 - k) + k;

    return numerator / denominator;
}

float GSmith(float NdotV, float LdotN, float roughness) {
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

vec3 tonemapReinhard(const vec3 color)
{
    return color / (color + vec3(1.0));
}

vec3 lightDirectContribution(const PerLightDotProducts dots, float NdotV, float roughness, float metallic, const vec3 albedo, const vec3 specular_color)
{
    // Cook-Torrance specular BRDF
    float D = DGgx(dots.HdotN, roughness);
    float G = GSmith(NdotV, dots.LdotN, roughness);
    vec3 F = FSchlick(dots.HdotV, specular_color);
    vec3 numer = D * G * F;
    float denom = 4 * NdotV * dots.LdotN + 0.001;
    vec3 specular = numer / denom;
    vec3 kS = F;

    // Lambertian BRDF
    vec3 diffuse = diffuseLambertian(albedo);
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic; // Metallic materials have ~0 diffuse contribution

    return kD * diffuse + specular;
}

void computePerLightDotProducts(const vec3 L, const vec3 N, const vec3 V, inout PerLightDotProducts dots)
{
    const vec3 H = normalize(L + V);  // Halfway vector
    dots.LdotN = clamp(dot(L, N), 0, 1);
    dots.HdotV = clamp(dot(H, V), 0, 1);
    dots.HdotN = clamp(dot(H, N), 0, 1);
}

vec3 dirLightDirectContribution(int index, const vec3 N, const vec3 V, float NdotV, float roughness, float metallic, vec3 albedo, vec3 specular_color)
{
    vec3 L = -normalize(dirlights[index].direction);
    PerLightDotProducts dots;
    computePerLightDotProducts(L, N, V, dots); 

    // Radiance
    vec3 radiance = dirlights[index].intensity * dots.LdotN * dirlights[index].color;
    return radiance * lightDirectContribution(dots, NdotV, roughness, metallic, albedo, specular_color);
}

vec3 pointLightDirectContribution(int index, const vec3 N, const vec3 V, float NdotV, vec3 surface_position, float roughness, float metallic, vec3 albedo, vec3 specular_color)
{
    vec3 StoL = pointlights[index].position - surface_position; // Surface to light
    vec3 L = normalize(StoL);
    float dist = length(StoL);
    PerLightDotProducts dots;
    computePerLightDotProducts(L, N, V, dots); 

    // Radiance
    vec3 radiance = pointlights[index].intensity * dots.LdotN * pointlights[index].color * falloff(dist, pointlights[index].radius);
    return radiance * lightDirectContribution(dots, NdotV, roughness, metallic, albedo, specular_color);
}

void main() {
    /*if (bayerMatrix[int(gl_FragCoord.x) % 4][int(gl_FragCoord.y) % 4] > opacity)
    {
        discard;
    }*/
    vec3 position = geom_position;
    
    // Albedo
    vec3 alb = albedo * geom_color;
    alb = use_albedo_texture ? texture(albedo_tex, geom_uv).rgb : alb;
    
    int edge_flag = int(round(geom_wire));
    // edge_flag == 0: wireframe is not rendered
    // edge_flag == 1: wireframe is rendered
    // edge_flag == 2: wireframe is rendered in PURPLE
    // edge_flag == 3: wireframe is rendered in PINK
    // Draw a wireframe if it's set as visible globally or if the edge is set as visible
    if (edge_flag > 0) {
        // Find the smallest distance
        float d = min(dist.x, dist.y);
        d = min(d, dist.z);
        float thickness = edge_flag > 1 ? 2.0 * wireframe.thickness : wireframe.thickness;
        if (d < thickness)
        {
            float mix_val = smoothstep(thickness - 1, thickness + 1, d);
            vec3 wcolor = edge_flag == 3 ? PINK : (edge_flag == 2 ? PURPLE : (wireframe.show ? wireframe.color : alb));
            alb = mix(wcolor, alb, mix_val);
        }
    }

    // Metallic
    float met = metallic;
    met = use_metallic_texture ? texture(metallic_tex, geom_uv).r * met: met;
    met = clamp(met, 0.0, 1.0);

    // Normal
    vec3 N = use_normal_texture ? geom_tbn * (texture(normal_tex, geom_uv).rgb * 2.0 - 1.0) : geom_normal;
    N = normalize(N);
    
    // Roughness
    float rou = roughness;
    rou = use_roughness_texture ? texture(roughness_tex, geom_uv).r * rou : rou;
    rou = clamp(rou, 0.04, 1.0);
    
    // Surface to eye
    vec3 V = normalize(vec3(eye_pos - position));
    
    // Specular color
    vec3 specular_color = vec3(0.04);
    specular_color = mix(specular_color, alb, met);

    // Direct lighting
    const float NdotV = clamp(dot(N, V), 0, 1);
    vec3 direct = vec3(0.0);
    for (int i = 0; i < min(num_dirlights, RCUBE_MAX_DIRLIGHTS); ++i)
    {
        direct += dirLightDirectContribution(i, N, V, NdotV, rou, met, alb, specular_color);
    }

    for (int i = 0; i < min(num_pointlights, RCUBE_MAX_POINTLIGHTS); ++i)
    {
        direct += pointLightDirectContribution(i, N, V, NdotV, position, rou, met, alb, specular_color);
    }

    // Indirect image-based lighting for ambient term
    vec3 indirect = vec3(0.03) * alb; // default ambient color for non-IBL setting
    if (use_image_based_lighting) {
        vec3 F = FSchlickRoughness(NdotV, specular_color, rou);
        vec3 kS = F;
        vec3 kD = 1.0 - kS;
        kD *= 1.0 - met;

        vec3 irradiance = texture(irradiance_map, N).rgb;
        vec3 diffuse = irradiance * alb;
        
        const float MAX_REFLECTION_LOD = 4.0;
        vec3 R = reflect(-V, N);
        vec3 prefiltered_color = textureLod(prefilter_map, R,  rou * MAX_REFLECTION_LOD).rgb;
        vec2 brdf  = texture(brdf_lut, vec2(NdotV, rou)).rg;
        vec3 specular = prefiltered_color * (F * brdf.x + brdf.y);

        indirect += (kD * diffuse + specular) * 1.0;
    }

    vec3 result = direct + indirect;
    vec4 final_color = vec4(result, opacity);

    // Output
#if RCUBE_RENDERPASS == 0
    out_color = final_color;
#elif RCUBE_RENDERPASS == 1
    float weight = clamp(pow(min(1.0, final_color.a * 10.0) + 0.01, 3.0) * 1e8 * 
                         pow(1.0 - gl_FragCoord.z * 0.9, 3.0), 1e-2, 3e3);
    accum = vec4(final_color.rgb * final_color.a, final_color.a) * weight;
    reveal = final_color.a;
#endif
}
)";

StandardMaterial::StandardMaterial() : ShaderMaterial("StandardMaterial")
{
    ForwardRenderSystemShaderManager::instance().create("StandardMaterial", StandardVertexShader,
                                     StandardGeometryShader, StandardFragmentShader, true);
    textures_.reserve(5);
    cubemaps_.reserve(2);
    state_.blend.enabled = true;
    state_.cull.enabled = false;
    state_.depth.test = true;
    state_.depth.write = true;
    state_.depth.func = DepthFunc::Less;
    state_.dither = false;
    state_.stencil.test = true;
    state_.stencil.write = 0xFF;
    state_.stencil.func = StencilFunc::Always;
    state_.stencil.func_ref = 1;
    state_.stencil.func_mask = 0xFF;
    state_.stencil.op_stencil_pass = StencilOp::Replace;
    state_.stencil.op_stencil_fail = StencilOp::Keep;
    state_.stencil.op_depth_fail = StencilOp::Keep;
}

void StandardMaterial::updateUniforms(std::shared_ptr<ShaderProgram> shader)
{
    shader->uniform("albedo").set(glm::pow(albedo, glm::vec3(2.2f)));
    shader->uniform("roughness").set(roughness);
    shader->uniform("metallic").set(metallic);
    shader->uniform("use_albedo_texture").set(albedo_texture != nullptr);
    shader->uniform("use_roughness_texture").set(roughness_texture != nullptr);
    shader->uniform("use_normal_texture").set(normal_texture != nullptr);
    shader->uniform("use_metallic_texture").set(metallic_texture != nullptr);
    shader->uniform("wireframe.show").set(wireframe);
    shader->uniform("wireframe.color").set(glm::pow(wireframe_color, glm::vec3(2.2f)));
    shader->uniform("wireframe.thickness").set(wireframe_thickness);
    shader->uniform("use_image_based_lighting")
        .set(image_based_lighting && ibl_irradiance != nullptr && ibl_prefilter != nullptr &&
             ibl_brdfLUT != nullptr);
    ShaderMaterial::updateUniforms(shader);
}

const std::vector<DrawCall::Texture2DInfo> StandardMaterial::textureSlots()
{
    textures_.clear();
    if (albedo_texture != nullptr)
    {
        textures_.push_back({albedo_texture->id(), 0});
    }
    if (roughness_texture != nullptr)
    {
        textures_.push_back({roughness_texture->id(), 1});
    }
    if (metallic_texture != nullptr)
    {
        textures_.push_back({metallic_texture->id(), 2});
    }
    if (normal_texture != nullptr)
    {
        textures_.push_back({normal_texture->id(), 3});
    }
    const bool use_ibl = image_based_lighting && ibl_irradiance != nullptr &&
                         ibl_prefilter != nullptr && ibl_brdfLUT != nullptr;
    if (use_ibl)
    {
        textures_.push_back({ibl_brdfLUT->id(), 4});
    }
    return textures_;
}

const std::vector<DrawCall::TextureCubemapInfo> StandardMaterial::cubemapSlots()
{
    cubemaps_.clear();
    const bool use_ibl = image_based_lighting && ibl_irradiance != nullptr &&
                         ibl_prefilter != nullptr && ibl_brdfLUT != nullptr;
    if (use_ibl)
    {
        cubemaps_.push_back({ibl_prefilter->id(), 5});
        cubemaps_.push_back({ibl_irradiance->id(), 6});
    }
    return cubemaps_;
}

void StandardMaterial::setIBLFromCamera(Camera *cam)
{
    if (cam != nullptr)
    {
        ibl_irradiance = cam->irradiance;
        ibl_prefilter = cam->prefilter;
        ibl_brdfLUT = cam->brdfLUT;
    }
}

void StandardMaterial::drawGUI()
{
    ImGui::Text("StandardMaterial");
    ImGui::Text("This material renders the object with a\nphysically-based material based on "
                "the\nmetallic-roughness model.");
    ImGui::Separator();
    ImGui::ColorEdit3("Albedo", glm::value_ptr(albedo));
    ImGui::SliderFloat("Roughness", &roughness, 0.04f, 1.f);
    ImGui::SliderFloat("Metallic", &metallic, 0.f, 1.f);
    ShaderMaterial::drawGUI();
    ImGui::Text("Wireframe");
    ImGui::Checkbox("Show", &wireframe);
    ImGui::InputFloat("Thickness", &wireframe_thickness);
    ImGui::ColorEdit3("Color", glm::value_ptr(wireframe_color));
    ImGui::Checkbox("Image-based lighting", &image_based_lighting);
}

} // namespace rcube