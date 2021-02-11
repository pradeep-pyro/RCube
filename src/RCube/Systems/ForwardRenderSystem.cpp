#include "RCube/Systems/ForwardRenderSystem.h"
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
#include "RCube/Systems/Shaders.h"
#include "glm/gtx/string_cast.hpp"

#define RCUBE_MAX_DIRECTIONAL_LIGHTS 5

#include <string>

const std::string StandardVertexShader =
    R"(
#version 450
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
#version 450
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
#version 420
in vec3 geom_position;
in vec3 geom_normal;
in vec2 geom_uv;
in vec3 geom_color;
in mat3 geom_tbn;
in float geom_wire;
noperspective in vec3 dist;

#define RCUBE_MAX_DIRLIGHTS 5
out vec4 out_color;

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

vec3 radianceDirLight(int index, float LdotN)
{
    return dirlights[index].intensity * LdotN * dirlights[index].color;
}

vec3 tonemapReinhard(const vec3 color)
{
    return color / (color + vec3(1.0));
}

void main() {
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
    float NdotV = max(dot(N, V), 0);
    // Specular color
    vec3 specular_color = vec3(0.04);
    specular_color = mix(specular_color, alb, met);
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
        float D = DGgx(HdotN, rou);
        float G = GSmith(NdotV, LdotN, rou);
        vec3 F = FSchlick(HdotV, specular_color);
        vec3 numer = D * G * F;
        float denom = 4 * NdotV * LdotN + 0.001;
        vec3 specular = numer / denom;
        vec3 kS = F;

        // Lambertian BRDF
        vec3 diffuse = diffuseLambertian(alb);
        vec3 kD = vec3(1.0) - kS;
        kD *= 1.0 - met; // Metallic materials have ~0 diffuse contribution

        //float visibility = 1.0 - shadow(i, position, LdotN);
        vec3 radiance = radianceDirLight(i, LdotN);
        direct += /*visibility * */ (kD * diffuse + specular) * radiance;
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

        indirect = (kD * diffuse + specular) * 1.0;
    }

    vec3 result = direct + indirect;

    // Output
    out_color = vec4(result, 1.0);
}
)";

namespace rcube
{

ForwardRenderSystem::ForwardRenderSystem(glm::ivec2 resolution, unsigned int msaa)
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

void ForwardRenderSystem::initialize()
{
    // HDR framebuffer
    framebuffer_hdr_ = Framebuffer::create();
    auto color = Texture2D::create(resolution_.x, resolution_.y, 1, TextureInternalFormat::RGB16F);
    framebuffer_hdr_->setColorAttachment(0, color);
    auto depth =
        Texture2D::create(resolution_.x, resolution_.y, 1, TextureInternalFormat::Depth32FStencil8);
    framebuffer_hdr_->setDepthStencilAttachment(depth);
    framebuffer_hdr_->setDrawBuffers({0});
    assert(framebuffer_hdr_->isComplete());

    // Lighting shader
    shader_standard_ = ShaderProgram::create(StandardVertexShader, StandardGeometryShader,
                                             StandardFragmentShader, true);

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
    // Initialize postprocess
    initializePostprocess();
}

void ForwardRenderSystem::setCameraUBO(const glm::vec3 &eye_pos, const glm::mat4 &world_to_view,
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

void ForwardRenderSystem::setDirectionalLightsUBO()
{

    std::vector<Entity> dirlights = getFilteredEntities({DirectionalLight::family()});
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
                dirlight_data_[k++] = dl->cast_shadow ? light_matrix[i][j] : 0.f;
            }
        }
    }
    const int num_lights = static_cast<int>(dirlights.size());
    ubo_dirlights_->setData(dirlight_data_.data(), dirlight_data_.size(), 0);
    ubo_dirlights_->setData(&num_lights, 1, RCUBE_MAX_DIRECTIONAL_LIGHTS * 24 * sizeof(float));
    ubo_dirlights_->bindBase(1);
}

void ForwardRenderSystem::initializePostprocess()
{
    // Bloom framebuffer to store brightness map
    auto brightness =
        Texture2D::create(resolution_[0] / 2, resolution_[1] / 2, 1, TextureInternalFormat::RGB16F);
    brightness->setFilterMode(TextureFilterMode::Linear);
    brightness->setWrapMode(TextureWrapMode::ClampToEdge);
    framebuffer_brightness_ = Framebuffer::create();
    framebuffer_brightness_->setColorAttachment(0, brightness);
    // Two pingpong HDR framebuffers for separable Gaussian blurring
    auto pingpong1 =
        Texture2D::create(resolution_[0] / 2, resolution_[1] / 2, 1, TextureInternalFormat::RGB16F);
    pingpong1->setFilterMode(TextureFilterMode::Linear);
    pingpong1->setWrapMode(TextureWrapMode::ClampToEdge);
    auto pingpong2 =
        Texture2D::create(resolution_[0] / 2, resolution_[1] / 2, 1, TextureInternalFormat::RGB16F);
    pingpong2->setFilterMode(TextureFilterMode::Linear);
    pingpong2->setWrapMode(TextureWrapMode::ClampToEdge);
    framebuffer_blur_[0] = Framebuffer::create();
    framebuffer_blur_[0]->setColorAttachment(0, pingpong1);
    framebuffer_blur_[1] = Framebuffer::create();
    framebuffer_blur_[1]->setColorAttachment(0, pingpong2);
    // HDR framebuffer for postprocessing
    framebuffer_pp_ = Framebuffer::create();
    auto pp_color =
        Texture2D::create(resolution_.x, resolution_.y, 1, TextureInternalFormat::RGB16F);
    framebuffer_pp_->setColorAttachment(0, pp_color);

    shader_blur_ = common::fullScreenQuadShader(shaders::BlurFragmentShader);
    shader_brightness_ = common::fullScreenQuadShader(shaders::BrightnessFragmentShader);
    shader_pp_ = common::fullScreenQuadShader(shaders::PostprocessFragmentShader);
}

void ForwardRenderSystem::cleanup()
{
    renderer_.cleanup();
}

unsigned int ForwardRenderSystem::priority() const
{
    return 300;
}

void ForwardRenderSystem::update(bool)
{
    const auto &dirlight_entities = registered_entities_[filters_[0]];
    const auto &camera_entities = registered_entities_[filters_[1]];

    // Render all drawable entities
    setDirectionalLightsUBO();
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

        // Render passes
        renderOpaqueGeometry(cam);
        renderTransparentGeometry(cam);
        postprocessPass(cam);
        finalPass(cam);
    }
}

void ForwardRenderSystem::renderOpaqueGeometry(Camera *cam)
{
    RenderTarget rt;
    rt.clear_color_buffer = true;
    rt.clear_depth_buffer = true;
    rt.clear_stencil_buffer = true;
    rt.framebuffer = framebuffer_hdr_->id();
    rt.viewport_origin = glm::ivec2(0, 0);
    rt.viewport_size = resolution_;

    RenderSettings state;
    state.depth.test = true;
    state.depth.write = true;
    state.stencil.test = false;
    /*state.stencil.func = StencilFunc::Always;
    state.stencil.func_ref = 1;
    state.stencil.func_mask = 0xFF;
    state.stencil.write = 0xFF;
    state.stencil.op_pass = StencilOp::Replace;
    state.stencil.op_depth_fail = StencilOp::Replace;
    state.stencil.op_stencil_fail = StencilOp::Replace;*/
    state.cull.enabled = false;

    std::vector<DrawCall> drawcalls;
    const auto &drawable_entities = registered_entities_[filters_[2]];
    drawcalls.reserve(drawable_entities.size());
    for (const auto &drawable_entity : drawable_entities)
    {
        Drawable *dr = world_->getComponent<Drawable>(drawable_entity);
        if (!dr->visible)
        {
            continue;
        }
        Mesh *mesh = dr->mesh.get();
        Transform *tr = world_->getComponent<Transform>(drawable_entity);
        Material *mat = world_->getComponent<Material>(drawable_entity);

        DrawCall dc;
        dc.settings = state;
        dc.mesh = GLRenderer::getDrawCallMeshInfo(dr->mesh);
        if (mat->albedo_texture != nullptr)
        {
            dc.textures.push_back({mat->albedo_texture->id(), 0});
        }
        if (mat->roughness_texture != nullptr)
        {
            dc.textures.push_back({mat->roughness_texture->id(), 1});
        }
        if (mat->metallic_texture != nullptr)
        {
            dc.textures.push_back({mat->metallic_texture->id(), 2});
        }
        if (mat->normal_texture != nullptr)
        {
            dc.textures.push_back({mat->normal_texture->id(), 3});
        }
        const bool use_ibl =
            cam->irradiance != nullptr && cam->prefilter != nullptr && cam->brdfLUT != nullptr;
        if (use_ibl)
        {
            dc.textures.push_back({cam->brdfLUT->id(), 4});
            dc.cubemaps.push_back({cam->prefilter->id(), 5});
            dc.cubemaps.push_back({cam->irradiance->id(), 6});
        }
        dc.shader = shader_standard_;
        dc.update_uniforms = [use_ibl, tr, mat, cam](std::shared_ptr<ShaderProgram> shader) {
            shader->uniform("albedo").set(glm::pow(mat->albedo, glm::vec3(2.2f)));
            shader->uniform("roughness").set(mat->roughness);
            shader->uniform("metallic").set(mat->metallic);
            shader->uniform("use_albedo_texture").set(mat->albedo_texture != nullptr);
            shader->uniform("use_roughness_texture").set(mat->roughness_texture != nullptr);
            shader->uniform("use_normal_texture").set(mat->normal_texture != nullptr);
            shader->uniform("use_metallic_texture").set(mat->metallic_texture != nullptr);
            shader->uniform("model_matrix").set(tr->worldTransform());
            shader->uniform("normal_matrix").set(glm::mat3(tr->worldTransform()));
            shader->uniform("wireframe.show").set(mat->wireframe);
            shader->uniform("wireframe.color").set(glm::pow(mat->wireframe_color, glm::vec3(2.2f)));
            shader->uniform("wireframe.thickness").set(mat->wireframe_thickness);
            shader->uniform("use_image_based_lighting").set(use_ibl);
        };
        drawcalls.push_back(dc);
    }
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
        drawcalls.push_back(dc_skybox);
    }
    renderer_.draw(rt, drawcalls);
}

void ForwardRenderSystem::renderTransparentGeometry(Camera *cam)
{
}

void ForwardRenderSystem::postprocessPass(Camera *cam)
{
    RenderSettings state;
    state.stencil.test = false;
    state.stencil.write = false;
    state.depth.test = false;
    state.depth.write = false;
    DrawCall::MeshInfo mi = GLRenderer::getDrawCallMeshInfo(renderer_.fullscreenQuadMesh());
    {
        // Pass 1: extract bright colors from the HDR framebuffer
        DrawCall dc;
        dc.settings = state;
        dc.settings.depth.test = false;
        dc.settings.depth.write = false;
        dc.mesh = mi;
        dc.shader = shader_brightness_;
        dc.update_uniforms = [&](std::shared_ptr<ShaderProgram> shader) {
            shader->uniform("bloom_threshold").set(cam->bloom_threshold);
        };
        dc.textures.push_back({framebuffer_hdr_->colorAttachment(0)->id(), 0});
        RenderTarget rt;
        rt.clear_color_buffer = true;
        rt.clear_depth_buffer = false;
        rt.clear_stencil_buffer = false;
        rt.viewport_origin = glm::ivec2(0, 0);
        rt.viewport_size = resolution_ / 2;
        rt.framebuffer = framebuffer_brightness_->id();
        renderer_.draw(rt, {dc});
    }
    {
        // Pass 2: Gaussian Blur
        const size_t blur_amount = 10;
        bool horizontal = true;
        for (size_t i = 0; i < blur_amount; ++i)
        {
            DrawCall dc;
            dc.settings = state;
            dc.settings.depth.test = false;
            dc.settings.depth.write = false;
            dc.mesh = mi;
            dc.shader = shader_blur_;
            dc.textures.push_back(
                {i == 0 ? framebuffer_brightness_->colorAttachment(0)->id()
                        : framebuffer_blur_[int(!horizontal)]->colorAttachment(0)->id(),
                 0});
            dc.update_uniforms = [horizontal](std::shared_ptr<ShaderProgram> shader) {
                shader->uniform("horizontal").set(horizontal);
            };
            RenderTarget rt;
            rt.clear_color_buffer = true;
            rt.clear_depth_buffer = false;
            rt.clear_stencil_buffer = false;
            rt.viewport_origin = glm::ivec2(0, 0);
            rt.viewport_size = resolution_ / 2;
            rt.framebuffer = framebuffer_blur_[int(horizontal)]->id();
            renderer_.draw(rt, {dc});
            horizontal = !horizontal;
        }
    }
    {
        // Pass 3: Apply postprocess effects: bloom and tonemap
        DrawCall dc;
        dc.settings = state;
        dc.settings.depth.test = false;
        dc.settings.depth.write = false;
        dc.mesh = mi;
        dc.shader = shader_pp_;
        dc.textures.push_back({framebuffer_hdr_->colorAttachment(0)->id(), 0});
        dc.textures.push_back({framebuffer_blur_[0]->colorAttachment(0)->id(), 1});
        RenderTarget rt;
        rt.clear_color_buffer = true;
        rt.clear_depth_buffer = false;
        rt.clear_stencil_buffer = false;
        rt.viewport_origin = glm::ivec2(0, 0);
        rt.viewport_size = resolution_;
        rt.framebuffer = framebuffer_pp_->id();
        renderer_.draw(rt, {dc});
    }
}

void ForwardRenderSystem::finalPass(Camera *cam)
{
    RenderTarget rt_screen;
    rt_screen.framebuffer = 0;
    rt_screen.clear_color_buffer = false;
    rt_screen.clear_depth_buffer = false;
    rt_screen.viewport_origin = cam->viewport_origin;
    rt_screen.viewport_size = cam->viewport_size;
    renderer_.drawTexture(rt_screen, framebuffer_pp_->colorAttachment(0));
}
} // namespace rcube