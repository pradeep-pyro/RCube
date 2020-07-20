#include "RCube/Systems/DeferredRenderSystem.h"
#include "RCube/Components/BaseLight.h"
#include "RCube/Components/Camera.h"
#include "RCube/Components/Drawable.h"
#include "RCube/Components/Transform.h"
#include "RCube/Core/Graphics/Materials/PhysicallyBasedMaterial.h"
#include "RCube/Core/Graphics/OpenGL/CheckGLError.h"
#include "RCube/Core/Graphics/OpenGL/Light.h"
#include "RCube/Systems/RenderSystem.h"
#include "glm/gtx/string_cast.hpp"

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

out vec3 frag_position;
out vec2 frag_uv;
out vec3 frag_color;
out vec3 frag_normal;
out mat3 tbn;

uniform mat4 model_matrix;
uniform mat3 normal_matrix;

void main()
{
    vec4 world_pos = model_matrix * vec4(position, 1.0);
    frag_position = world_pos.xyz; 
    frag_uv = uv;
    frag_color = color;
    frag_normal = normal_matrix * normal;
    gl_Position = projection_matrix * view_matrix * world_pos;
    // Tangent basis
    vec3 T = normalize(vec3(model_matrix * vec4(tangent, 0.0)));
    vec3 B = cross(normal, T);
    tbn = mat3(T, B, normal);
}
)";

const std::string GBufferFragmentShader =
    R"(
#version 420
in vec3 frag_position;
in vec3 frag_normal;
in vec2 frag_uv;
in vec3 frag_color;
in mat3 tbn;

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

void main() {
    vec3 alb = albedo * frag_color;
    alb = use_albedo_texture ? texture(albedo_tex, frag_uv).rgb : alb;
    g_albedo = alb;

    float met = metallic;
    met = use_metallic_texture ? texture(metallic_tex, frag_uv).r * met: met;
    met = clamp(met, 0.0, 1.0);
    vec3 N = use_normal_texture ? tbn * (texture(normal_tex, frag_uv).rgb * 2.0 - 1.0) : frag_normal;
    N = normalize(N);
    g_normal.rgb = N;
    g_normal.a = met;

    float rou = roughness;
    rou = use_roughness_texture ? texture(roughness_tex, frag_uv).r * rou : rou;
    rou = clamp(rou, 0.04, 1.0);
    g_position.rgb = frag_position;
    g_position.a = rou;
}
)";

const std::string PBRLightingPassShader = R"(
#version 420

#define MAX_LIGHTS 99

out vec4 out_color;

layout(binding=0) uniform sampler2D g_position_roughness;
layout(binding=1) uniform sampler2D g_normal_metallic;
layout(binding=2) uniform sampler2D g_albedo;

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

struct Light {
    vec4 position;
    vec4 direction_radius;
    vec4 color_coneangle;
};

layout (std140, binding=2) uniform Lights {
    Light lights[MAX_LIGHTS];
    int num_lights;
};

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
    for (int i = 0; i < min(num_lights, MAX_LIGHTS); ++i)
    {
        vec3 L;          // Surface to light
        float att = 1.0; // Light attenuation

        if (close(lights[i].position.w, 0.0)) { // is directional?
            L = lights[i].position.xyz;
        }
        else {
            L = lights[i].position.xyz - position;
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
        direct += (kD * diffuse + specular) * radiance * dot(L, N);
        //direct += diffuse * radiance * dot(L, N);
    }

    // Indirect image-based lighting for ambient term
    vec3 indirect = vec3(0.03) * albedo; // default ambient color for non-IBL setting
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

        indirect = (kD * diffuse + specular) * 1.0;
    }
    vec3 result = direct + indirect;

    // Tone mapping using Reinhard operator
    //result = result / (result + vec3(1.0));

    // Output
    result = pow(result, vec3(1.0 / 2.2));
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
    ComponentMask light_filter;
    light_filter.set(BaseLight::family());
    light_filter.set(Transform::family());
    addFilter(light_filter);

    ComponentMask camera_filter;
    camera_filter.set(Camera::family());
    camera_filter.set(Transform::family());
    addFilter(camera_filter);

    ComponentMask renderable_filter;
    renderable_filter.set(Transform::family());
    renderable_filter.set(Drawable::family());
    addFilter(renderable_filter);
}

void DeferredRenderSystem::initialize()
{
    gbuffer_ = createGBuffer(resolution_.x, resolution_.y);
    gbuffer_shader_ = ShaderProgram::create(GBufferVertexShader, GBufferFragmentShader, true);
    lighting_shader_ = makeEffect(PBRLightingPassShader);
    framebuffer_hdr_ = Framebuffer::create();
    auto color = Texture2D::create(resolution_.x, resolution_.y, 1, TextureInternalFormat::RGB16F);
    framebuffer_hdr_->setColorAttachment(0, color);
    auto depth =
        Texture2D::create(resolution_.x, resolution_.y, 1, TextureInternalFormat::Depth32FStencil8);
    framebuffer_hdr_->setDepthStencilAttachment(depth);
    framebuffer_hdr_->setDrawBuffers({0});
    assert(framebuffer_hdr_->isComplete());
}

void DeferredRenderSystem::cleanup()
{
    renderer_.cleanup();
}

void DeferredRenderSystem::update(bool force)
{
    const auto &light_entities = registered_entities_[filters_[0]];
    const auto &camera_entities = registered_entities_[filters_[1]];
    const auto &renderable_entities = registered_entities_[filters_[2]];

    // Set lights
    std::vector<Light> lights;
    lights.reserve(light_entities.size());
    for (const auto &e : light_entities)
    {
        BaseLight *light_comp = world_->getComponent<BaseLight>(e);
        Transform *transform_comp = world_->getComponent<Transform>(e);
        Light light = light_comp->light();
        light.position = transform_comp->worldPosition();
        lights.push_back(light);
    }
    renderer_.setLights(lights);

    // Render all drawable entities
    for (const auto &camera_entity : camera_entities)
    {
        Camera *cam = world_->getComponent<Camera>(camera_entity);
        Transform *tr = world_->getComponent<Transform>(camera_entity);
        if (!cam->rendering)
        {
            continue;
        }

        // Set camera & lights
        renderer_.setCamera(tr->worldPosition(), cam->world_to_view, cam->view_to_projection,
                            cam->projection_to_viewport);

        // Set and clear draw area
        gbuffer_->useForWrite();
        renderer_.resize(0, 0, resolution_.x, resolution_.y);

        // Geometry pass
        glEnable(GL_STENCIL_TEST);
        glEnable(GL_DEPTH_TEST);
        glStencilMask(0xFF);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        glCullFace(GL_BACK);
        glDepthMask(GL_TRUE);
        glStencilMask(0xFF);
        glStencilFunc(GL_ALWAYS, 1, 0xFF);
        glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
        for (const auto &render_entity : renderable_entities)
        {
            Drawable *dr = world_->getComponent<Drawable>(render_entity);
            if (!dr->visible)
            {
                continue;
            }
            Mesh *mesh = dr->mesh.get();
            Transform *tr = world_->getComponent<Transform>(render_entity);
            auto pbr = std::dynamic_pointer_cast<PhysicallyBasedMaterial>(dr->material);
            const bool use_ibl = pbr->irradiance_map != nullptr && pbr->prefilter_map != nullptr &&
                                 pbr->brdf_map != nullptr;
            gbuffer_shader_->uniform("albedo").set(pbr->albedo);
            gbuffer_shader_->uniform("roughness").set(pbr->roughness);
            gbuffer_shader_->uniform("metallic").set(pbr->metallic);
            gbuffer_shader_->uniform("use_albedo_texture").set(pbr->albedo_texture != nullptr);
            gbuffer_shader_->uniform("use_roughness_texture")
                .set(pbr->roughness_texture != nullptr);
            gbuffer_shader_->uniform("use_normal_texture").set(pbr->normal_texture != nullptr);
            gbuffer_shader_->uniform("use_metallic_texture").set(pbr->metallic_texture != nullptr);
            if (pbr->albedo_texture != nullptr)
            {
                pbr->albedo_texture->use(0);
            }
            if (pbr->roughness_texture != nullptr)
            {
                pbr->roughness_texture->use(1);
            }
            if (pbr->metallic_texture != nullptr)
            {
                pbr->metallic_texture->use(2);
            }
            if (pbr->normal_texture != nullptr)
            {
                pbr->normal_texture->use(3);
            }
            renderer_.render(mesh, gbuffer_shader_.get(), tr->worldTransform());
        }
        gbuffer_->done();
        gbuffer_->blit(*framebuffer_hdr_, {0, 0}, resolution_, {0, 0}, resolution_, false, true,
                       true);
        framebuffer_hdr_->useForWrite();
        glClear(GL_COLOR_BUFFER_BIT);
        glDisable(GL_STENCIL_TEST);
        glEnable(GL_DEPTH_TEST);
        lighting_shader_->use();
        bool use_ibl = (cam->irradiance != nullptr) && (cam->prefilter != nullptr) &&
                       (cam->brdfLUT != nullptr);
        lighting_shader_->uniform("use_image_based_lighting").set(use_ibl);
        if (use_ibl)
        {
            cam->brdfLUT->use(4);
            cam->prefilter->use(5);
            cam->irradiance->use(6);
        }
        gbuffer_->colorAttachment(0)->use(0);
        gbuffer_->colorAttachment(1)->use(1);
        gbuffer_->colorAttachment(2)->use(2);
        renderer_.renderEffect(lighting_shader_.get(), gbuffer_.get());
        if (cam->use_skybox)
        {
            glEnable(GL_STENCIL_TEST);
            glDisable(GL_DEPTH_TEST);
            glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
            glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
            glStencilMask(0x00);
            renderer_.renderSkyBox(cam->skybox);
        }

        renderer_.resize(cam->viewport_origin.x, cam->viewport_origin.y, cam->viewport_size.x,
                         cam->viewport_size.y);
        renderer_.renderTextureToScreen(framebuffer_hdr_->colorAttachment(0));
    }
}

unsigned int DeferredRenderSystem::priority() const
{
    return 301;
}

} // namespace rcube