#include "RCube/Systems/RenderSystem.h"
#include "RCube/Components/BaseLight.h"
#include "RCube/Components/Camera.h"
#include "RCube/Components/Drawable.h"
#include "RCube/Components/Transform.h"
#include "RCube/Core/Graphics/OpenGL/CheckGLError.h"
#include "RCube/Core/Graphics/OpenGL/Light.h"
#include "glm/gtx/string_cast.hpp"

namespace rcube
{

RenderSystem::RenderSystem(glm::ivec2 resolution, unsigned int msaa)
    : resolution_(resolution), msaa_(msaa)
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

unsigned int RenderSystem::priority() const
{
    return 300;
}

void RenderSystem::initialize()
{
    framebufferms_ = Framebuffer::create(resolution_[0], resolution_[1]);
    framebufferms_->addColorAttachment(TextureInternalFormat::RGBA16, 1, msaa_);
    framebufferms_->addDepthAttachment(TextureInternalFormat::Depth24Stencil8, msaa_);
    assert(framebufferms_->isComplete());
    framebuffer_ = Framebuffer::create(resolution_[0], resolution_[1]);
    framebuffer_->addColorAttachment(TextureInternalFormat::RGBA16);
    framebuffer_->addDepthAttachment(TextureInternalFormat::Depth24Stencil8);
    assert(framebuffer_->isComplete());
    effect_framebuffer_ = Framebuffer::create(resolution_[0], resolution_[1]);
    effect_framebuffer_->addColorAttachment(TextureInternalFormat::RGBA16);
    assert(effect_framebuffer_->isComplete());
    renderer.initialize();
    checkGLError();
}

void RenderSystem::cleanup()
{
    const auto &renderable_entities = registered_entities_[filters_[2]];
    for (const auto &e : renderable_entities)
    {
        Drawable *dr = world_->getComponent<Drawable>(e);
        // dr->material->shader()->release();
        dr->material->release();
    }
    const auto &camera_entities = registered_entities_[filters_[1]];
    for (const auto &e : camera_entities)
    {
        Camera *cam = world_->getComponent<Camera>(e);
        for (auto item : cam->postprocess)
        {
            item->release();
        }
    }

    renderer.cleanup();
}

void RenderSystem::update(bool /* force */)
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
    renderer.setLights(lights);

    auto render_fbo = msaa_ > 0 ? framebufferms_ : framebuffer_;

    // Render all drawable entities
    for (const auto &camera_entity : camera_entities)
    {
        Camera *cam = world_->getComponent<Camera>(camera_entity);
        if (!cam->rendering)
        {
            continue;
        }

        // Set and clear draw area
        render_fbo->use();
        renderer.resize(0, 0, render_fbo->width(), render_fbo->height());
        renderer.setClearColor(cam->background_color);
        renderer.clear(true, true, true);

        // set camera & lights
        renderer.setCamera(cam->world_to_view, cam->view_to_projection,
                           cam->projection_to_viewport);

        // Draw all opaque
        for (const auto &render_entity : renderable_entities)
        {
            Drawable *dr = world_->getComponent<Drawable>(render_entity);
            if (!dr->visible)
            {
                continue;
            }
            assert(dr->material != nullptr && dr->mesh != nullptr);
            Transform *tr = world_->getComponent<Transform>(render_entity);
            if (dr->material->renderPriority() == RenderPriority::Opaque)
            {
                renderer.render(dr->mesh.get(), dr->material.get(), tr->worldTransform());
            }
        }

        // Draw skybox if in use
        if (cam->use_skybox)
        {
            renderer.renderSkyBox(cam->skybox);
        }

        // Blit multisample framebuffer content to regular framebuffer
        if (msaa_ > 0)
        {
            framebufferms_->blit(*framebuffer_);
        }

        // Postprocess
        Framebuffer *curr_fbo = framebuffer_.get();
        if (cam->postprocess.size() > 0)
        {
            for (size_t i = 0; i < cam->postprocess.size(); ++i)
            {
                ShaderProgram *curr_effect = cam->postprocess[i].get();
                Framebuffer *prev_fbo =
                    (i % 2 == 0) ? framebuffer_.get() : effect_framebuffer_.get();
                curr_fbo = (i % 2 == 1) ? framebuffer_.get() : effect_framebuffer_.get();
                curr_fbo->use();
                renderer.renderEffect(curr_effect, prev_fbo);
            }
        }
        renderer.resize(cam->viewport_origin.x, cam->viewport_origin.y, cam->viewport_size.x,
                        cam->viewport_size.y);
        renderer.renderTextureToScreen(curr_fbo->colorAttachment(0));
    }
}

} // namespace rcube
