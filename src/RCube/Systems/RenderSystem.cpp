#include "RCube/Systems/RenderSystem.h"
#include "RCube/Components/BaseLight.h"
#include "RCube/Components/Camera.h"
#include "RCube/Components/DirectionalLight.h"
#include "RCube/Components/Drawable.h"
#include "RCube/Components/Material.h"
#include "RCube/Components/Transform.h"
#include "RCube/Core/Graphics/OpenGL/CheckGLError.h"
#include "RCube/Core/Graphics/OpenGL/CommonMesh.h"
#include "RCube/Core/Graphics/OpenGL/CommonShader.h"
#include "RCube/Core/Graphics/OpenGL/Light.h"
#include "RCube/Systems/RenderSystem.h"
#include "glm/gtx/string_cast.hpp"

#define RCUBE_MAX_DIRECTIONAL_LIGHTS 50

namespace rcube
{

const std::string GBufferVertexShader =
    R"(
#version 420
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 uv;
layout (location = 3) in vec3 color;
layout (location = 4) in vec3 tangent;

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

uniform mat4 model_matrix;
uniform mat3 normal_matrix;

void main()
{
    vec4 world_pos = model_matrix * vec4(position, 1.0);
    vert_position = world_pos.xyz;
    vert_uv = uv;
    vert_color = color;
    vert_normal = normal_matrix * normal;
    gl_Position = projection_matrix * view_matrix * world_pos;
    // Tangent basis
    vec3 T = normalize(vec3(model_matrix * vec4(tangent, 0.0)));
    vec3 B = cross(normal, T);
    vert_tbn = mat3(T, B, normal);
}
)";

const static std::string GBufferGeometryShader =
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
    gl_Position = gl_in[0].gl_Position;
    EmitVertex();

    // Emit vertex 2
    dist = vec3(0, hb, 0);
    geom_position = vert_position[1];
    geom_normal = vert_normal[1];
    geom_uv = vert_uv[1];
    geom_color = vert_color[1];
    geom_tbn = vert_tbn[1];
    gl_Position = gl_in[1].gl_Position;
    EmitVertex();

    // Emit vertex 3
    dist = vec3(0, 0, hc);
    geom_position = vert_position[2];
    geom_normal = vert_normal[2];
    geom_uv = vert_uv[2];
    geom_color = vert_color[2];
    geom_tbn = vert_tbn[2];
    gl_Position = gl_in[2].gl_Position;
    EmitVertex();
    EndPrimitive();
}
)";

const std::string GBufferFragmentShader =
    R"(
#version 420
in vec3 geom_position;
in vec3 geom_normal;
in vec2 geom_uv;
in vec3 geom_color;
in mat3 geom_tbn;
noperspective in vec3 dist;

layout(location=0) out vec4 g_position;
layout(location=1) out vec4 g_normal;
layout(location=2) out vec3 g_albedo;

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

struct Wireframe {
    bool show;
    vec3 color;
    float thickness;
};
uniform Wireframe wireframe;
uniform bool show_wireframe;

void main() {
    vec3 alb = albedo * geom_color;
    alb = use_albedo_texture ? texture(albedo_tex, geom_uv).rgb : alb;
    
    if (wireframe.show) {
        // Find the smallest distance
        float d = min(dist.x, dist.y);
        d = min(d, dist.z);
        if (d < wireframe.thickness)
        {
            float mix_val = smoothstep(wireframe.thickness - 1.0, wireframe.thickness + 1.0, d);
            alb = mix(wireframe.color, alb, mix_val);
        }
    }
    
    g_albedo = alb;

    float met = metallic;
    met = use_metallic_texture ? texture(metallic_tex, geom_uv).r * met: met;
    met = clamp(met, 0.0, 1.0);
    vec3 N = use_normal_texture ? geom_tbn * (texture(normal_tex, geom_uv).rgb * 2.0 - 1.0) : geom_normal;
    N = normalize(N);
    g_normal.rgb = N;
    g_normal.a = met;

    float rou = roughness;
    rou = use_roughness_texture ? texture(roughness_tex, geom_uv).r * rou : rou;
    rou = clamp(rou, 0.04, 1.0);
    g_position.rgb = geom_position;
    g_position.a = rou;
}
)";

const std::string PBRLightingPassShader = R"(
#version 420

#define RCUBE_MAX_DIRLIGHTS 50

#define MAX_LIGHTS 99

out vec4 out_color;

layout(binding=0) uniform sampler2D g_position_roughness;
layout(binding=1) uniform sampler2D g_normal_metallic;
layout(binding=2) uniform sampler2D g_albedo;

layout(binding=3) uniform sampler2D shadow_atlas;

layout(binding=4) uniform sampler2D brdf_lut;
layout(binding=5) uniform samplerCube prefilter_map;
layout(binding=6) uniform samplerCube irradiance_map;
uniform bool use_image_based_lighting;

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

/*
struct Light {
    vec4 position;
    vec4 direction_radius;
    vec4 color_coneangle;
};

layout (std140, binding=2) uniform Lights {
    Light lights[MAX_LIGHTS];
    int num_lights;
};

uniform mat4 light_viewproj;
*/

in vec2 v_texcoord;

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

float shadow(int index, vec3 world_pos, float LdotN)
{
    if (dirlights[index].cast_shadow <= 1e-6)
    {
        return 0.0;
    }
    const float bias = 0.05;
    vec4 shadow_pos = dirlights[index].view_proj * vec4(world_pos, 1.0);
    // perform perspective divide
    vec3 shadow_coords = shadow_pos.xyz / shadow_pos.w;
    shadow_coords = shadow_coords * 0.5 + 0.5; 

    float closest = texture(shadow_atlas, shadow_coords.xy).r; 
    float current = shadow_coords.z;
    float shadow = current - bias > closest ? 1.0 : 0.0;
    if (shadow_coords.z > 1.0)
    {
        shadow = 0.0;
    }
    return shadow;
}

vec3 radianceDirLight(int index, float LdotN)
{
    return dirlights[index].intensity * LdotN * dirlights[index].color;
}

vec3 tonemapReinhard(const vec3 color)
{
    return color / (color + vec3(1.0));
}

void main()
{
    // From G-buffer
    vec4 position_roughness = texture(g_position_roughness, v_texcoord);
    vec3 position = position_roughness.rgb;
    float roughness = position_roughness.a;
    vec4 normal_metallic = texture(g_normal_metallic, v_texcoord);
    vec3 N = normal_metallic.rgb;
    float metallic = normal_metallic.a;
    vec3 albedo = texture(g_albedo, v_texcoord).rgb;

    // Surface to eye
    vec3 V = normalize(vec3(eye_pos - position));
    float NdotV = max(dot(N, V), 0);
    // Specular color
    vec3 specular_color = vec3(0.04);
    specular_color = mix(specular_color, albedo, metallic);
    vec3 direct = vec3(0.0);
    for (int i = 0; i < min(num_dirlights, RCUBE_MAX_DIRLIGHTS); ++i)
    {
        vec3 L = -normalize(dirlights[i].direction);  // Surface to light
        // Useful values to precompute
        vec3 H = normalize(L + V);  // Halfway vector
        float LdotN = clamp(dot(L, N), 0, 1);
        float NdotV = clamp(dot(N, V), 0, 1);
        float HdotV = clamp(dot(H, V), 0, 1);
        float HdotN = clamp(dot(H, N), 0, 1);

        // Cook-Torrance specular BRDF
        float D = DGgx(HdotN, roughness);
        float G = GSmith(NdotV, LdotN, roughness);
        vec3 F = FSchlick(HdotV, specular_color);
        vec3 numer = D * G * F;
        float denom = 4 * NdotV * LdotN + 0.001;
        vec3 specular = numer / denom;
        vec3 kS = F;

        // Lambertian BRDF
        vec3 diffuse = diffuseLambertian(albedo);
        vec3 kD = vec3(1.0) - kS;
        kD *= 1.0 - metallic; // Metallic materials have ~0 diffuse contribution

        float visibility = 1.0 - shadow(i, position, LdotN);
        vec3 radiance = radianceDirLight(i, LdotN);
        direct += visibility * (kD * diffuse + specular) * radiance;
    }

    // Indirect image-based lighting for ambient term
    vec3 indirect = vec3(0.03) * albedo; // default ambient color for non-IBL setting
    if (use_image_based_lighting) {
        vec3 F = FSchlickRoughness(NdotV, specular_color, roughness);
        vec3 kS = F;
        vec3 kD = 1.0 - kS;
        kD *= 1.0 - metallic;

        vec3 irradiance = texture(irradiance_map, N).rgb;
        vec3 diffuse = irradiance * albedo;
        
        const float MAX_REFLECTION_LOD = 4.0;
        vec3 R = reflect(-V, N);
        vec3 prefiltered_color = textureLod(prefilter_map, R,  roughness * MAX_REFLECTION_LOD).rgb;
        vec2 brdf  = texture(brdf_lut, vec2(NdotV, roughness)).rg;
        vec3 specular = prefiltered_color * (F * brdf.x + brdf.y);

        indirect = (kD * diffuse + specular) * 1.0;
    }

    vec3 result = direct + indirect;

    // Tone mapping
    result = tonemapReinhard(result);

    // Gamma correction
    result = pow(result, vec3(1.0 / 2.2));

    // Output
    out_color = vec4(result , 1.0);
}
)";

std::shared_ptr<Framebuffer> createGBuffer(size_t width, size_t height)
{
    auto fbo = Framebuffer::create();
    // Positions
    auto positions = Texture2D::create(width, height, 1, TextureInternalFormat::RGBA16F);
    positions->setFilterMode(TextureFilterMode::Nearest);
    fbo->setColorAttachment(0, positions);
    // Normals
    auto normals = Texture2D::create(width, height, 1, TextureInternalFormat::RGBA16F);
    normals->setFilterMode(TextureFilterMode::Nearest);
    fbo->setColorAttachment(1, normals);
    // Albedo
    auto albedo = Texture2D::create(width, height, 1, TextureInternalFormat::RGBA16);
    albedo->setFilterMode(TextureFilterMode::Nearest);
    fbo->setColorAttachment(2, albedo);
    // Depth
    auto depth_stencil =
        Texture2D::create(width, height, 1, TextureInternalFormat::Depth32FStencil8);
    fbo->setDepthStencilAttachment(depth_stencil);
    fbo->setDrawBuffers({0, 1, 2});
    assert(fbo->isComplete());
    return fbo;
}

DeferredRenderSystem::DeferredRenderSystem(glm::ivec2 resolution, unsigned int msaa)
    : resolution_(resolution)
{
    ComponentMask dirlight_filter;
    dirlight_filter.set(DirectionalLight::family());
    addFilter(dirlight_filter);

    ComponentMask camera_filter;
    camera_filter.set(Camera::family());
    camera_filter.set(Transform::family());
    addFilter(camera_filter);

    ComponentMask renderable_filter;
    renderable_filter.set(Transform::family());
    renderable_filter.set(Drawable::family());
    renderable_filter.set(Material::family());
    addFilter(renderable_filter);
}

void DeferredRenderSystem::initialize()
{
    // GBuffer
    gbuffer_ = createGBuffer(resolution_.x, resolution_.y);
    gbuffer_shader_ = ShaderProgram::create(GBufferVertexShader, GBufferGeometryShader,
                                            GBufferFragmentShader, true);

    // HDR framebuffer
    framebuffer_hdr_ = Framebuffer::create();
    auto color = Texture2D::create(resolution_.x, resolution_.y, 1, TextureInternalFormat::RGB16F);
    framebuffer_hdr_->setColorAttachment(0, color);
    auto depth =
        Texture2D::create(resolution_.x, resolution_.y, 1, TextureInternalFormat::Depth32FStencil8);
    framebuffer_hdr_->setDepthStencilAttachment(depth);
    framebuffer_hdr_->setDrawBuffers({0});
    assert(framebuffer_hdr_->isComplete());

    // Shadow mapping
    framebuffer_shadow_ = Framebuffer::create();
    shadow_atlas_ = Texture2D::create(1024, 1024, 1, TextureInternalFormat::Depth32F);
    shadow_atlas_->setFilterMode(TextureFilterMode::Nearest);
    shadow_atlas_->setWrapMode(TextureWrapMode::ClampToBorder);
    shadow_atlas_->setBorderColor(glm::vec4(1.0));
    framebuffer_shadow_->setDepthAttachment(shadow_atlas_);
    framebuffer_shadow_->setDrawBuffers({});
    glNamedFramebufferReadBuffer(framebuffer_shadow_->id(), GL_NONE);
    assert(framebuffer_shadow_->isComplete());
    shadow_shader_ = common::shadowMapShader();

    // Lighting shader
    lighting_shader_ = common::fullScreenQuadShader(PBRLightingPassShader);

    // UBOs
    // Three 4x4 matrices and one 3D vector
    ubo_camera_ = UniformBuffer::create(sizeof(glm::mat4) * 3 + sizeof(glm::vec3));
    // Each light has one 3D direction, one bool flag for shadow casting, one 3D color, one scalar,
    // one 4x4 matrix intensity: 8 floats Finally there one int (treated as float) for number of
    // directional lights
    ubo_dirlights_ =
        UniformBuffer::create(RCUBE_MAX_DIRECTIONAL_LIGHTS * 24 * sizeof(float) + sizeof(float));
    dirlight_data_.resize(RCUBE_MAX_DIRECTIONAL_LIGHTS * 24);
    // Initialize renderer
    renderer_.initialize();
}

void DeferredRenderSystem::cleanup()
{
    renderer_.cleanup();
}

void DeferredRenderSystem::setCameraUBO(const glm::vec3 &eye_pos, const glm::mat4 &world_to_view,
                                        const glm::mat4 &view_to_projection,
                                        const glm::mat4 &projection_to_viewport)
{
    const int float4x4_size = sizeof(glm::mat4);
    ubo_camera_->setData(glm::value_ptr(world_to_view), 16, 0);
    ubo_camera_->setData(glm::value_ptr(view_to_projection), 16, float4x4_size);
    ubo_camera_->setData(glm::value_ptr(projection_to_viewport), 16, 2 * float4x4_size);
    ubo_camera_->setData(glm::value_ptr(eye_pos), 3, 3 * float4x4_size);
    ubo_camera_->bindBase(0);
}

void DeferredRenderSystem::setDirectionalLightsUBO()
{
    
    std::vector<Entity> dirlights = getFilteredEntities({DirectionalLight::family()});;
    // Copy lights
    assert(dirlights.size() < RCUBE_MAX_DIRECTIONAL_LIGHTS);
    size_t k = 0;
    for (const Entity &l : dirlights)
    {
        DirectionalLight *dl = world_->getComponent<DirectionalLight>(l);
        const glm::vec3 dir = glm::normalize(dl->direction);
        const glm::vec3 &col = dl->color;
        // Light's viewproj matrix
        const glm::vec3 opp_dir = -dir;
        const glm::mat4 light_proj = glm::ortho<float>(-10, 10, -10, 10, -10, 20);
        const glm::vec3 up = glm::length(glm::cross(opp_dir, glm::vec3(0, 1, 0))) > 1e-6
                                 ? glm::vec3(0, 1, 0)
                                 : glm::vec3(1, 0, 0);
        const glm::mat4 light_view = glm::lookAt(opp_dir, glm::vec3(0, 0, 0), up);
        const glm::mat4 light_matrix = light_proj * light_view;
        dirlight_data_[k++] = dir.x;
        dirlight_data_[k++] = dir.y;
        dirlight_data_[k++] = dir.z;
        dirlight_data_[k++] = float(dl->cast_shadow);
        dirlight_data_[k++] = col.r;
        dirlight_data_[k++] = col.g;
        dirlight_data_[k++] = col.b;
        dirlight_data_[k++] = dl->intensity;
        for (int i = 0; i < 4; ++i)
        {
            for (int j = 0; j < 4; ++j)
            {
                dirlight_data_[k++] = dl->cast_shadow ? light_matrix[i][j] : 0.0;
            }
        }
    }
    const int num_lights = static_cast<int>(dirlights.size());
    ubo_dirlights_->setData(dirlight_data_.data(), dirlight_data_.size(), 0);
    ubo_dirlights_->setData(&num_lights, 1,
                            RCUBE_MAX_DIRECTIONAL_LIGHTS * 24 * sizeof(float));
    ubo_dirlights_->bindBase(1);
}

void DeferredRenderSystem::update(bool force)
{
    const auto &dirlight_entities = registered_entities_[filters_[0]];
    const auto &camera_entities = registered_entities_[filters_[1]];
    const auto &renderable_entities = registered_entities_[filters_[2]];

    // Shadow pass
    for (auto light : dirlight_entities)

    {
        DirectionalLight *dirL = world_->getComponent<DirectionalLight>(light);
        if (!dirL->cast_shadow)
        {
            continue;
        }
        RenderTarget rtsh;
        rtsh.framebuffer = framebuffer_shadow_->id();
        rtsh.clear_color_buffer = false;
        rtsh.clear_depth_buffer = true;
        rtsh.clear_stencil_buffer = false;
        rtsh.viewport_origin = dirL->shadowmap_origin;
        rtsh.viewport_size = dirL->shadowmap_size;
        assert(framebuffer_shadow_->isComplete());
        const glm::vec3 opp_dirL = -glm::normalize(dirL->direction);
        const glm::mat4 light_proj = glm::ortho<float>(-10, 10, -10, 10, -10, 20);
        const glm::mat4 light_view = glm::lookAt(opp_dirL, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
        const glm::mat4 light_matrix = light_proj * light_view;
        std::vector<DrawCall> dcsh;
        dcsh.reserve(dirlight_entities.size());
        for (auto renderable : renderable_entities)
        {
            const glm::mat4 model_matrix =
                world_->getComponent<Transform>(renderable)->worldTransform();
            DrawCall dc;
            dc.mesh =
                renderer_.getDrawCallMeshInfo(world_->getComponent<Drawable>(renderable)->mesh);
            dc.settings.depth.write = true;
            dc.settings.depth.test = true;
            dc.settings.cull.enabled = false;
            dc.shader = shadow_shader_;
            dc.update_uniforms = [&](std::shared_ptr<ShaderProgram> shader) {
                shader->uniform("light_matrix").set(light_matrix);
                shader->uniform("model_matrix").set(model_matrix);
            };
            dcsh.push_back(dc);
        }
        renderer_.draw(rtsh, dcsh);
    }

    // Render all drawable entities
    for (const auto &camera_entity : camera_entities)
    {
        Camera *cam = world_->getComponent<Camera>(camera_entity);
        Transform *tr = world_->getComponent<Transform>(camera_entity);
        if (!cam->rendering)
        {
            continue;
        }

        // Set camera
        setCameraUBO(tr->worldPosition(), cam->world_to_view, cam->view_to_projection,
                     cam->projection_to_viewport);

        //////////////////////////////////////////////////////////////////////////////////////
        // Geometry pass
        //////////////////////////////////////////////////////////////////////////////////////
        RenderTarget rt_geom_pass;
        rt_geom_pass.clear_color_buffer = true;
        rt_geom_pass.clear_depth_buffer = true;
        rt_geom_pass.clear_stencil_buffer = true;
        rt_geom_pass.framebuffer = gbuffer_->id();
        rt_geom_pass.viewport_origin = glm::ivec2(0, 0);
        rt_geom_pass.viewport_size = resolution_;

        RenderSettings state;
        state.depth.test = true;
        state.depth.write = true;
        state.stencil.test = true;
        state.stencil.func = StencilFunc::Always;
        state.stencil.func_ref = 1;
        state.stencil.func_mask = 0xFF;
        state.stencil.write = 0xFF;
        state.stencil.op_pass = StencilOp::Replace;
        state.stencil.op_depth_fail = StencilOp::Replace;
        state.stencil.op_stencil_fail = StencilOp::Replace;
        state.cull.enabled = false;

        std::vector<DrawCall> drawcalls_geom_pass;
        drawcalls_geom_pass.reserve(renderable_entities.size());
        for (const auto &render_entity : renderable_entities)
        {
            Drawable *dr = world_->getComponent<Drawable>(render_entity);
            if (!dr->visible)
            {
                continue;
            }
            Mesh *mesh = dr->mesh.get();
            Transform *tr = world_->getComponent<Transform>(render_entity);
            auto pbr = world_->getComponent<Material>(render_entity);

            DrawCall dc;
            dc.settings = state;
            dc.mesh = GLRenderer::getDrawCallMeshInfo(dr->mesh);
            if (pbr->albedo_texture != nullptr)
            {
                dc.textures.push_back({pbr->albedo_texture->id(), 0});
            }
            if (pbr->roughness_texture != nullptr)
            {
                dc.textures.push_back({pbr->roughness_texture->id(), 1});
            }
            if (pbr->metallic_texture != nullptr)
            {
                dc.textures.push_back({pbr->metallic_texture->id(), 2});
            }
            if (pbr->normal_texture != nullptr)
            {
                dc.textures.push_back({pbr->normal_texture->id(), 3});
            }
            dc.shader = gbuffer_shader_;
            dc.update_uniforms = [tr, pbr](std::shared_ptr<ShaderProgram> shader) {
                shader->uniform("albedo").set(pbr->albedo);
                shader->uniform("roughness").set(pbr->roughness);
                shader->uniform("metallic").set(pbr->metallic);
                shader->uniform("use_albedo_texture").set(pbr->albedo_texture != nullptr);
                shader->uniform("use_roughness_texture").set(pbr->roughness_texture != nullptr);
                shader->uniform("use_normal_texture").set(pbr->normal_texture != nullptr);
                shader->uniform("use_metallic_texture").set(pbr->metallic_texture != nullptr);
                shader->uniform("model_matrix").set(tr->worldTransform());
                shader->uniform("normal_matrix").set(glm::mat3(tr->worldTransform()));
                shader->uniform("wireframe.show").set(pbr->wireframe);
                shader->uniform("wireframe.color").set(pbr->wireframe_color);
                shader->uniform("wireframe.thickness").set(pbr->wireframe_thickness);
            };
            drawcalls_geom_pass.push_back(dc);
        }
        renderer_.draw(rt_geom_pass, drawcalls_geom_pass);
        gbuffer_->blit(framebuffer_hdr_, {0, 0}, resolution_, {0, 0}, resolution_, false, true,
                       true);

        //////////////////////////////////////////////////////////////////////////////////////
        // Lighting pass
        //////////////////////////////////////////////////////////////////////////////////////
        std::vector<DrawCall> dcs;
        RenderTarget rtl;
        rtl.framebuffer = framebuffer_hdr_->id();
        rtl.clear_color_buffer = true;
        rtl.clear_depth_buffer = false;
        rtl.clear_stencil_buffer = false;
        rtl.viewport_origin = glm::ivec2(0, 0);
        rtl.viewport_size = resolution_;
        if (!cam->use_skybox || cam->skybox == nullptr)
        {
            rtl.clear_color[0] = cam->background_color[0];
            rtl.clear_color[1] = cam->background_color[1];
            rtl.clear_color[2] = cam->background_color[2];
        }
        DrawCall dc_light;
        RenderSettings &sl = dc_light.settings;
        sl.depth.test = true;
        sl.depth.write = false;
        sl.stencil.test = false;
        sl.cull.enabled = false;
        dc_light.shader = lighting_shader_;
        dc_light.textures.push_back({gbuffer_->colorAttachment(0)->id(), 0});
        dc_light.textures.push_back({gbuffer_->colorAttachment(1)->id(), 1});
        dc_light.textures.push_back({gbuffer_->colorAttachment(2)->id(), 2});
        dc_light.textures.push_back({shadow_atlas_->id(), 3});
        const bool use_ibl =
            cam->irradiance != nullptr && cam->prefilter != nullptr && cam->brdfLUT != nullptr;
        if (use_ibl)
        {
            dc_light.textures.push_back({cam->brdfLUT->id(), 4});
            dc_light.cubemaps.push_back({cam->prefilter->id(), 5});
            dc_light.cubemaps.push_back({cam->irradiance->id(), 6});
        }
        dc_light.update_uniforms = [&](std::shared_ptr<ShaderProgram> shader) {
            shader->uniform("use_image_based_lighting").set(use_ibl);
        };
        dc_light.mesh = GLRenderer::getDrawCallMeshInfo(renderer_.fullscreenQuadMesh());
        dcs.push_back(dc_light);

        // Draw skybox
        if (cam->use_skybox && !cam->orthographic)
        {
            DrawCall dc_skybox;
            RenderSettings &s = dc_skybox.settings;
            s.depth.write = false;
            s.depth.func = DepthFunc::LessOrEqual;
            s.stencil.test = true;
            s.stencil.op_stencil_fail = StencilOp::Keep;
            s.stencil.op_depth_fail = StencilOp::Keep;
            s.stencil.op_pass = StencilOp::Keep;
            s.stencil.func = StencilFunc::NotEqual;
            s.stencil.func_ref = 1;
            s.stencil.func_mask = 0xFF;
            s.stencil.write = 0x00;
            dc_skybox.mesh = GLRenderer::getDrawCallMeshInfo(renderer_.skyboxMesh());
            dc_skybox.shader = renderer_.skyboxShader();
            dc_skybox.cubemaps.push_back({cam->skybox->id(), 0});
            dcs.push_back(dc_skybox);
        }
        setDirectionalLightsUBO();
        renderer_.draw(rtl, dcs);
        RenderTarget rtsc;
        rtsc.viewport_origin = cam->viewport_origin;
        rtsc.viewport_size = cam->viewport_size;
        rtsc.framebuffer = 0;
        rtsc.clear_color_buffer = false;
        rtsc.clear_depth_buffer = false;
        rtsc.clear_stencil_buffer = false;
        DrawCall dcsc;
        dcsc.settings.depth.test = false;
        dcsc.textures.push_back({framebuffer_hdr_->colorAttachment(0)->id(), 0});
        dcsc.mesh = GLRenderer::getDrawCallMeshInfo(renderer_.fullscreenQuadMesh());
        dcsc.shader = renderer_.fullscreenQuadShader();
        dcsc.settings.cull.enabled = false;
        renderer_.draw(rtsc, {dcsc});
    }
} // namespace rcube

unsigned int DeferredRenderSystem::priority() const
{
    return 301;
}

} // namespace rcube