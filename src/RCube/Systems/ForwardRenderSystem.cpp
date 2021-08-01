#include "RCube/Systems/ForwardRenderSystem.h"
#include "RCube/Components/Camera.h"
#include "RCube/Components/DirectionalLight.h"
#include "RCube/Components/Drawable.h"
#include "RCube/Components/ForwardMaterial.h"
#include "RCube/Components/PointLight.h"
#include "RCube/Components/Transform.h"
#include "RCube/Core/Graphics/OpenGL/CheckGLError.h"
#include "RCube/Core/Graphics/OpenGL/CommonMesh.h"
#include "RCube/Core/Graphics/OpenGL/CommonShader.h"
#include "RCube/Core/Graphics/OpenGL/Light.h"
#include "RCube/Core/Graphics/ShaderManager.h"
#include "RCube/Systems/Shaders.h"
#include "RCubeViewer/Components/Name.h"
#include "glm/gtx/string_cast.hpp"
#include <string>

namespace rcube
{

DrawCall makeDrawCall(Drawable *dr, ShaderMaterial *material, Transform *tr, ForwardRenderPass pass)
{
    DrawCall dc;
    dc.settings = material->state();
    dc.mesh = GLRenderer::getDrawCallMeshInfo(dr->mesh);
    dc.textures = material->textureSlots();
    dc.cubemaps = material->cubemapSlots();
    dc.shader = ForwardRenderSystemShaderManager::instance().get(material->name(), pass);
    dc.update_uniforms = [dc, material, tr](std::shared_ptr<ShaderProgram>) {
        dc.shader->uniform("model_matrix").set(tr->worldTransform());
        Uniform nor_mat;
        if (dc.shader->hasUniform("normal_matrix", nor_mat))
        {
            nor_mat.set(glm::mat3(tr->worldTransform()));
        }
        material->updateUniforms(dc.shader);
    };
    return dc;
}

ForwardRenderSystem::ForwardRenderSystem(glm::ivec2 resolution, unsigned int msaa)
    : resolution_(resolution), msaa_(msaa)
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
    renderable_filter.set(ForwardMaterial::family());
    addFilter(renderable_filter);

    ComponentMask pointlight_filter;
    pointlight_filter.set(PointLight::family());
    pointlight_filter.set(Transform::family());
    addFilter(pointlight_filter);
}

void ForwardRenderSystem::initialize()
{
    // HDR framebuffer
    framebuffer_hdr_ = Framebuffer::create();
    auto color = Texture2D::create(resolution_.x, resolution_.y, 1, TextureInternalFormat::RGB16F);
    auto depth =
        Texture2D::create(resolution_.x, resolution_.y, 1, TextureInternalFormat::Depth32FStencil8);
    framebuffer_hdr_->setColorAttachment(0, color);
    framebuffer_hdr_->setDepthStencilAttachment(depth);
    framebuffer_hdr_->setDrawBuffers({0});
    assert(framebuffer_hdr_->isComplete());

    // Create multisampled framebuffer if needed
    if (msaa_ > 0)
    {
        framebuffer_hdr_ms_ = Framebuffer::create();
        auto color_ms =
            Texture2D::createMS(resolution_.x, resolution_.y, msaa_, TextureInternalFormat::RGB16F);
        auto depth_ms = Texture2D::createMS(resolution_.x, resolution_.y, msaa_,
                                            TextureInternalFormat::Depth32FStencil8);
        framebuffer_hdr_ms_->setColorAttachment(0, color_ms);
        framebuffer_hdr_ms_->setDepthStencilAttachment(depth_ms);
        framebuffer_hdr_ms_->setDrawBuffers({0});
        assert(framebuffer_hdr_ms_->isComplete());
    }

    // UBOs
    // Three 4x4 matrices and one 3D vector
    ubo_camera_ = UniformBuffer::create(sizeof(glm::mat4) * 3 + sizeof(glm::vec3));
    // Directional lights: Each light has one 3D direction, one bool flag for shadow casting, one 3D
    // color, one scalar, one 4x4 matrix intensity. Finally there is one int (treated as float) for
    // number of directional lights
    ubo_dirlights_ =
        UniformBuffer::create(RCUBE_MAX_DIRECTIONAL_LIGHTS * 24 * sizeof(float) + sizeof(float));
    dirlight_data_.resize(RCUBE_MAX_DIRECTIONAL_LIGHTS * 24);
    // Point lights: Each light has one 3D position, one bool flag for shadow casting, one 3D color,
    // one float for radius, one float for intensity and 3 empty floats for padding. Finally there
    // is one int (treated as float) for number of directional lights
    ubo_pointlights_ =
        UniformBuffer::create(RCUBE_MAX_POINT_LIGHTS * 12 * sizeof(float) + sizeof(float));
    pointlight_data_.resize(RCUBE_MAX_POINT_LIGHTS * 12);

    // Shadows
    framebuffer_shadow_ = Framebuffer::create();
    shader_shadow_ = common::shadowMapShader();
    shadow_atlas_ = Texture2D::create(8000, 8000, 1, TextureInternalFormat::Depth32F);
    shadow_atlas_->setFilterMode(TextureFilterMode::Nearest);
    shadow_atlas_->setWrapMode(TextureWrapMode::ClampToBorder);
    shadow_atlas_->setBorderColor(glm::vec4(1.0));
    framebuffer_shadow_->setDrawBuffers({});
    framebuffer_shadow_->setReadBuffer(GL_NONE);
    framebuffer_shadow_->setDepthAttachment(shadow_atlas_);
    glNamedFramebufferReadBuffer(framebuffer_shadow_->id(), GL_NONE);
    assert(framebuffer_shadow_->isComplete());

    // For picking
    framebuffer_pick_ = Framebuffer::create();
    auto obj_id_tex =
        Texture2D::create(resolution_[0], resolution_[1], 1, TextureInternalFormat::RG32UI);
    framebuffer_pick_->setColorAttachment(0, obj_id_tex);
    framebuffer_pick_->setDepthAttachment(
        Texture2D::create(resolution_.x, resolution_.y, 1, TextureInternalFormat::Depth32F));
    framebuffer_pick_->setDrawBuffers({0});
    assert(framebuffer_pick_->isComplete());
    shader_picking_ = common::uniqueColorShader();

    // Transparency
    wboit_.initialize(resolution_, depth);

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
        const glm::vec3 dir = glm::normalize(dl->direction());
        const glm::vec3 &col = dl->color();
        // Light's viewproj matrix
        /*const glm::vec3 opp_dir = -dir;
        const glm::mat4 light_proj = glm::ortho<float>(-10, 10, -10, 10, -10, 20);
        const glm::vec3 up = glm::length(glm::cross(opp_dir, glm::vec3(0, 1, 0))) > 1e-6
                                 ? glm::vec3(0, 1, 0)
                                 : glm::vec3(1, 0, 0);
        const glm::mat4 light_view = glm::lookAt(opp_dir, glm::vec3(0, 0, 0), up);
        const glm::mat4 light_matrix = light_proj * light_view;*/
        const glm::mat4 light_matrix = dl->viewProjectionMatrix(-1, 1, -1, 1, 0.1f, 3.f);
        dirlight_data_[k++] = dir.x;
        dirlight_data_[k++] = dir.y;
        dirlight_data_[k++] = dir.z;
        dirlight_data_[k++] = float(dl->castShadow());
        dirlight_data_[k++] = col.r;
        dirlight_data_[k++] = col.g;
        dirlight_data_[k++] = col.b;
        dirlight_data_[k++] = dl->intensity();
        for (int i = 0; i < 4; ++i)
        {
            for (int j = 0; j < 4; ++j)
            {
                dirlight_data_[k++] = light_matrix[i][j];
            }
        }
    }
    const int num_lights = static_cast<int>(dirlights.size());
    ubo_dirlights_->setData(dirlight_data_.data(), dirlight_data_.size(), 0);
    ubo_dirlights_->setData(&num_lights, 1, RCUBE_MAX_DIRECTIONAL_LIGHTS * 24 * sizeof(float));
    ubo_dirlights_->bindBase(1);
}

void ForwardRenderSystem::setPointLightsUBO()
{
    std::vector<Entity> pointlights =
        getFilteredEntities({PointLight::family(), Transform::family()});
    // Copy lights
    assert(pointlights.size() < RCUBE_MAX_POINT_LIGHTS);
    size_t k = 0;
    for (const Entity &l : pointlights)
    {
        PointLight *pl = world_->getComponent<PointLight>(l);
        Transform *tr = world_->getComponent<Transform>(l);
        const glm::vec3 &pos = tr->worldPosition();
        float cast_shadow = static_cast<float>(pl->castShadow());
        const glm::vec3 &col = pl->color();
        float radius = pl->radius();
        pointlight_data_[k++] = pos.x;
        pointlight_data_[k++] = pos.y;
        pointlight_data_[k++] = pos.z;
        pointlight_data_[k++] = cast_shadow;
        pointlight_data_[k++] = col.x;
        pointlight_data_[k++] = col.y;
        pointlight_data_[k++] = col.z;
        pointlight_data_[k++] = radius;
        pointlight_data_[k++] = pl->intensity();
        pointlight_data_[k++] = 0.f; // padding
        pointlight_data_[k++] = 0.f;
        pointlight_data_[k++] = 0.f;
    }
    const int num_lights = static_cast<int>(pointlights.size());
    ubo_pointlights_->setData(pointlight_data_.data(), pointlight_data_.size(), 0);
    ubo_pointlights_->setData(&num_lights, 1, RCUBE_MAX_POINT_LIGHTS * 12 * sizeof(float));
    ubo_pointlights_->bindBase(2);
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

void ForwardRenderSystem::shadowMapPass()
{
    RenderTarget rt;
    rt.clear_color_buffer = false;
    rt.clear_stencil_buffer = false;
    rt.clear_depth_buffer = true;
    rt.framebuffer = framebuffer_shadow_->id();
    rt.viewport_origin = glm::ivec2(0, 0);
    rt.viewport_size = glm::ivec2(shadow_atlas_->width(), shadow_atlas_->height());

    std::vector<Entity> dirlights = getFilteredEntities({DirectionalLight::family()});
    const auto &drawable_entities =
        getFilteredEntities({Transform::family(), Drawable::family(), ForwardMaterial::family()});

    for (const Entity &l : dirlights)
    {
        DirectionalLight *dl = world_->getComponent<DirectionalLight>(l);
        if (!dl->castShadow())
        {
            continue;
        }
        // Light's viewproj matrix
        const glm::mat4 light_matrix = dl->viewProjectionMatrix(-1, 1, -1, 1, 0.1f, 3.f);
        std::vector<DrawCall> dcs;
        dcs.reserve(drawable_entities.size());
        for (const Entity &drawable_entity : drawable_entities)
        {
            Drawable *dr = world_->getComponent<Drawable>(drawable_entity);
            if (!dr->visible && !dr->cast_shadow)
            {
                continue;
            }
            Transform *tr = world_->getComponent<Transform>(drawable_entity);
            DrawCall dc;
            dc.settings.stencil.test = false;
            dc.settings.stencil.write = 0xFF;
            dc.settings.depth.test = true;
            dc.settings.depth.write = true;
            dc.mesh = GLRenderer::getDrawCallMeshInfo(dr->mesh);
            dc.shader = shader_shadow_;
            dc.update_uniforms = [light_matrix, tr](std::shared_ptr<ShaderProgram> sh) {
                const glm::mat4 &wt = tr->worldTransform();
                sh->uniform("model_matrix").set(wt);
                sh->uniform("light_matrix").set(light_matrix);
            };

            dcs.push_back(dc);
        }
        renderer_.draw(rt, dcs);
    }
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
    const auto &camera_entities = registered_entities_[filters_[1]];

    // Render all drawable entities
    setDirectionalLightsUBO();
    setPointLightsUBO();

    // Shadow pass
    shadowMapPass();

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
        opaqueGeometryPass(cam);
        // Resolve MSAA framebuffer if needed
        if (msaa_ > 0)
        {
            framebuffer_hdr_ms_->blit(framebuffer_hdr_, {0, 0}, resolution_, {0, 0}, resolution_);
        }
        transparentGeometryPass(cam);
        pickFBOPass(cam);
        postprocessPass(cam);
        finalPass(cam);
    }
}

void ForwardRenderSystem::opaqueGeometryPass(Camera *cam)
{
    RenderTarget rt;
    rt.clear_color_buffer = true;
    rt.clear_depth_buffer = true;
    rt.clear_stencil_buffer = true;
    rt.framebuffer = msaa_ > 0 ? framebuffer_hdr_ms_->id() : framebuffer_hdr_->id();
    rt.viewport_origin = glm::ivec2(0, 0);
    rt.viewport_size = resolution_;

    RenderSettings state;
    state.depth.test = true;
    state.depth.write = true;
    state.depth.func = DepthFunc::Less;
    state.stencil.test = false;
    state.cull.enabled = true;
    state.cull.mode = Cull::Back;
    state.dither = false;

    std::vector<DrawCall> drawcalls;
    const auto &drawable_entities =
        getFilteredEntities({Transform::family(), Drawable::family(), ForwardMaterial::family()});
    drawcalls.reserve(drawable_entities.size());
    for (Entity drawable_entity : drawable_entities)
    {
        Drawable *dr = world_->getComponent<Drawable>(drawable_entity);
        if (!dr->visible)
        {
            continue;
        }
        Mesh *mesh = dr->mesh.get();
        Transform *tr = world_->getComponent<Transform>(drawable_entity);
        ForwardMaterial *mat = world_->getComponent<ForwardMaterial>(drawable_entity);
        if (mat->shader == nullptr)
        {
            continue;
        }
        if (mat->opacity < 0.9999f)
        {
            continue;
        }
        // Add other shader passes to the drawcalls if they exist
        ShaderMaterial *sh = mat->shader.get();
        while (sh != nullptr)
        {
            drawcalls.push_back(makeDrawCall(dr, sh, tr, ForwardRenderPass::Opaque));
            drawcalls.back().textures.push_back({shadow_atlas_->id(), 10});
            sh = sh->next_pass.get();
        }
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
        s.stencil.op_stencil_pass = StencilOp::Keep;
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

void ForwardRenderSystem::transparentGeometryPass(Camera *cam)
{
    RenderTarget rt;
    RenderSettings state;
    rt.clear_color = {glm::vec4(0.f), glm::vec4(1.f)};
    rt.clear_depth_buffer = false;
    rt.clear_stencil_buffer = false;
    rt.framebuffer = wboit_.getFramebuffer()->id();
    rt.viewport_origin = glm::ivec2(0, 0);
    rt.viewport_size = resolution_;

    state.depth.test = true;
    state.depth.write = false;
    state.stencil.test = true;
    state.cull.enabled = false;
    state.blend.enabled = true;
    state.blend.blend.resize(2);
    state.blend.blend[0].color_src = BlendFunc::One;
    state.blend.blend[0].alpha_src = BlendFunc::One;
    state.blend.blend[0].color_dst = BlendFunc::One;
    state.blend.blend[0].alpha_dst = BlendFunc::One;
    state.blend.blend[1].color_src = BlendFunc::Zero;
    state.blend.blend[1].alpha_src = BlendFunc::Zero;
    state.blend.blend[1].color_dst = BlendFunc::OneMinusSrcColor;
    state.blend.blend[1].alpha_dst = BlendFunc::OneMinusSrcColor;
    state.blend.equation = BlendEq::Add;

    std::vector<DrawCall> drawcalls;
    const auto &drawable_entities =
        getFilteredEntities({Transform::family(), Drawable::family(), ForwardMaterial::family()});
    drawcalls.reserve(drawable_entities.size());
    for (Entity drawable_entity : drawable_entities)
    {
        Drawable *dr = world_->getComponent<Drawable>(drawable_entity);
        if (!dr->visible)
        {
            continue;
        }
        ForwardMaterial *mat = world_->getComponent<ForwardMaterial>(drawable_entity);
        if (mat->shader == nullptr)
        {
            continue;
        }
        if (mat->opacity >= 0.9999f)
        {
            continue;
        }
        Mesh *mesh = dr->mesh.get();
        Transform *tr = world_->getComponent<Transform>(drawable_entity);
        ShaderMaterial *sh = mat->shader.get();
        DrawCall dc = makeDrawCall(dr, sh, tr, ForwardRenderPass::Transparent);
        dc.ignore_settings = true;
        drawcalls.push_back(dc);
        renderer_.draw(rt, {dc});
    }
    if (drawcalls.empty())
    {
        return;
    }
    //renderer_.draw(rt, drawcalls);

    // Composite pass
    rt = RenderTarget();
    state = RenderSettings();
    wboit_.prepareCompositePass(framebuffer_hdr_, rt, state);
    std::shared_ptr<ShaderProgram> composite_shader = wboit_.getCompositeShader();
    drawcalls.clear();
    for (Entity drawable_entity : drawable_entities)
    {
        Drawable *dr = world_->getComponent<Drawable>(drawable_entity);
        if (!dr->visible)
        {
            continue;
        }
        ForwardMaterial *mat = world_->getComponent<ForwardMaterial>(drawable_entity);
        if (mat->shader == nullptr)
        {
            continue;
        }
        if (mat->opacity >= 0.9999f)
        {
            continue;
        }
        DrawCall dc;
        dc.mesh = GLRenderer::getDrawCallMeshInfo(renderer_.fullscreenQuadMesh());
        dc.shader = composite_shader;
        dc.textures.push_back(DrawCall::Texture2DInfo{wboit_.getAccumTexture()->id(), 0});
        dc.textures.push_back(DrawCall::Texture2DInfo{wboit_.getRevealageTexture()->id(), 1});
        drawcalls.push_back(dc);
        renderer_.draw(rt, {dc});
    }
}

void ForwardRenderSystem::pickFBOPass(Camera *cam)
{
    if (!pick_pass_)
    {
        return;
    }
    RenderTarget rt;
    rt.clear_color = {glm::ivec2(-1, -1)}; // Clear texture channels to -1
    rt.clear_depth_buffer = true;
    rt.clear_stencil_buffer = false;
    rt.framebuffer = framebuffer_pick_->id();
    rt.viewport_origin = glm::ivec2(0, 0);
    rt.viewport_size = resolution_;

    RenderSettings state;
    state.depth.test = true;
    state.depth.write = true;
    state.stencil.test = false;
    state.cull.enabled = false;

    const auto &drawable_entities =
        getFilteredEntities({Transform::family(), Drawable::family(), ForwardMaterial::family()});

    std::vector<DrawCall> drawcalls;
    drawcalls.reserve(drawable_entities.size());
    for (Entity drawable_entity : drawable_entities)
    {
        Drawable *dr = world_->getComponent<Drawable>(drawable_entity);
        if (!dr->visible)
        {
            continue;
        }
        DrawCall dc;
        DrawCall::MeshInfo mi = GLRenderer::getDrawCallMeshInfo(dr->mesh);
        dc.mesh = mi;
        Transform *tr = world_->getComponent<Transform>(drawable_entity);
        dc.shader = shader_picking_;
        const int id = static_cast<int>(drawable_entity.id());
        dc.update_uniforms = [tr, id](std::shared_ptr<ShaderProgram> shader) {
            shader->uniform("model_matrix").set(tr->worldTransform());
            shader->uniform("id").set(id);
        };
        drawcalls.push_back(dc);
    }
    renderer_.draw(rt, drawcalls);
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
        dc.update_uniforms = [cam](std::shared_ptr<ShaderProgram> shader) {
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

const std::string WBOITCompositeFragmentShader = R"(
#version 450

in vec2 v_texcoord;
out vec4 out_color;

layout (binding=0) uniform sampler2D accum;
layout (binding=1) uniform sampler2D reveal;

const float EPS = 0.0001f;

bool isApproximatelyEqual(float a, float b)
{
	return abs(a - b) <= (abs(a) < abs(b) ? abs(b) : abs(a)) * EPS;
}

float max3(vec3 v) 
{
	return max(max(v.x, v.y), v.z);
}

void main()
{
	// fragment revealage
	float revealage = texture(reveal, v_texcoord).r;
	
	// save the blending and color texture fetch cost if there is not a transparent fragment
	if (isApproximatelyEqual(revealage, 1.0f)) 
		discard;
 
	// fragment color
	vec4 accumulation = texture(accum, v_texcoord);
	
	// suppress overflow
	if (isinf(max3(abs(accumulation.rgb)))) 
    {
		accumulation.rgb = vec3(accumulation.a);
    }

	// prevent floating point precision bug
	vec3 average_color = accumulation.rgb / max(accumulation.a, EPS);

	// blend pixels
	out_color = vec4(average_color, 1.0f - revealage);
}
)";

void WeightedBlendedOITManager::initialize(glm::ivec2 resolution,
                                           std::shared_ptr<Texture2D> opaque_depth)
{
    fbo_ = Framebuffer::create();
    accum_tex_ = Texture2D::create(resolution[0], resolution[1], 1, TextureInternalFormat::RGBA16F);
    accum_tex_->setFilterMode(TextureFilterMode::Linear);
    revealage_tex_ =
        Texture2D::create(resolution[0], resolution[1], 1, TextureInternalFormat::R16F);
    revealage_tex_->setFilterMode(TextureFilterMode::Linear);
    fbo_->setColorAttachment(0, accum_tex_);
    fbo_->setColorAttachment(1, revealage_tex_);
    fbo_->setDepthAttachment(opaque_depth);
    fbo_->setDrawBuffers({0, 1});
    resolution_ = resolution;
    assert(fbo_->isComplete());

    composite_shader_ = common::fullScreenQuadShader(WBOITCompositeFragmentShader);
}

std::shared_ptr<Framebuffer> WeightedBlendedOITManager::getFramebuffer()
{
    return fbo_;
}

std::shared_ptr<Texture2D> WeightedBlendedOITManager::getAccumTexture()
{
    return accum_tex_;
}

std::shared_ptr<Texture2D> WeightedBlendedOITManager::getRevealageTexture()
{
    return revealage_tex_;
}

std::shared_ptr<ShaderProgram> WeightedBlendedOITManager::getCompositeShader()
{
    return composite_shader_;
}

void WeightedBlendedOITManager::prepareTransparentPass(RenderTarget &rt, RenderSettings &state)
{
    rt.clear_color = {glm::vec4(0.f), glm::vec4(1.f)};
    rt.clear_depth_buffer = false;
    rt.clear_stencil_buffer = false;
    rt.framebuffer = getFramebuffer()->id();
    rt.viewport_origin = glm::ivec2(0, 0);
    rt.viewport_size = resolution_;

    state.depth.test = true;
    state.depth.write = false;
    state.stencil.test = true;
    state.cull.enabled = false;
    state.blend.enabled = true;
    state.blend.blend.resize(2);
    state.blend.blend[0].color_src = BlendFunc::One;
    state.blend.blend[0].alpha_src = BlendFunc::One;
    state.blend.blend[0].color_dst = BlendFunc::One;
    state.blend.blend[0].alpha_dst = BlendFunc::One;
    state.blend.blend[1].color_src = BlendFunc::Zero;
    state.blend.blend[1].alpha_src = BlendFunc::Zero;
    state.blend.blend[1].color_dst = BlendFunc::OneMinusSrcColor;
    state.blend.blend[1].alpha_dst = BlendFunc::OneMinusSrcColor;
    state.blend.equation = BlendEq::Add;
}

void WeightedBlendedOITManager::prepareCompositePass(std::shared_ptr<Framebuffer> opaque_fbo,
                                                     RenderTarget &rt, RenderSettings &state)
{
    rt.framebuffer = opaque_fbo->id();
    rt.clear_color = {};
    rt.clear_depth = false;
    rt.clear_stencil = false;
    rt.viewport_origin = glm::ivec2(0, 0);
    rt.viewport_size = resolution_;

    state.depth.test = true;
    state.depth.func = DepthFunc::Always;
    state.blend.enabled = true;
    state.blend.blend.resize(1);
    state.blend.blend[0].alpha_src = BlendFunc::SrcAlpha;
    state.blend.blend[0].alpha_dst = BlendFunc::OneMinusSrcAlpha;
    state.blend.blend[0].color_src = BlendFunc::SrcAlpha;
    state.blend.blend[0].color_dst = BlendFunc::OneMinusSrcAlpha;
}

void WeightedBlendedOITManager::cleanup()
{
    fbo_->release();
    accum_tex_->release();
    revealage_tex_->release();
}

} // namespace rcube