#include "RenderSystem.h"
#include "../components/Drawable.h"
#include "../components/Transform.h"
#include "../components/Camera.h"
#include "../components/BaseLight.h"
#include "../render/Light.h"
#include "glm/gtx/string_cast.hpp"
#include "../render/checkglerror.h"

namespace rcube {

RenderSystem::RenderSystem() {
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

void RenderSystem::initialize() {
    checkGLError();
    renderer.initialize();
    checkGLError();

    // Compile shaders
    const auto &renderable_entities = registered_entities_[filters_[2]];
    for (const auto &e : renderable_entities) {
        Drawable *dr = world_->getComponent<Drawable>(e);
        dr->material->initialize();
    }
    checkGLError();

    const auto &camera_entities = registered_entities_[filters_[1]];
    for (const auto &e : camera_entities) {
        Camera *cam = world_->getComponent<Camera>(e);
        for (auto eff : cam->postprocess) {
            eff->initialize();
        }
    }
    checkGLError();
}

void RenderSystem::cleanup() {
    const auto &renderable_entities = registered_entities_[filters_[2]];
    for (const auto &e : renderable_entities) {
        Drawable *dr = world_->getComponent<Drawable>(e);
        dr->mesh->release();
        dr->material->shader()->release();
    }
    const auto &camera_entities = registered_entities_[filters_[1]];
    for (const auto &e : camera_entities) {
        Camera *cam = world_->getComponent<Camera>(e);
        cam->initFBO();
        for (auto item : cam->postprocess) {
            item->shader()->release();
            item->result->release();
        }
    }

    renderer.cleanup();
}

void RenderSystem::update(bool /* force */) {
    const auto &light_entities = registered_entities_[filters_[0]];
    const auto &camera_entities = registered_entities_[filters_[1]];
    const auto &renderable_entities = registered_entities_[filters_[2]];

    std::vector<Light> lights;
    lights.reserve(light_entities.size());
    for (const auto &e : light_entities) {
        BaseLight *light_comp = world_->getComponent<BaseLight>(e);
        Transform *transform_comp = world_->getComponent<Transform>(e);
        Light light = light_comp->light();
        light.position = transform_comp->worldPosition();
        lights.push_back(light);
    }

    // Render all drawable entities
    for (const auto &camera_entity : camera_entities) {
        Camera *cam = world_->getComponent<Camera>(camera_entity);
        checkGLError();

        // Set and clear draw area
        cam->initFBO(); // no-op if already initialized
        cam->framebuffer->use();
        renderer.resize(0, 0, cam->framebuffer->width(), cam->framebuffer->height());
        renderer.setClearColor(cam->background_color);
        renderer.clear(true, true, true);
        checkGLError();

        // set camera & lights
        renderer.setLightsCamera(lights, cam->world_to_view, cam->view_to_projection, cam->projection_to_viewport);
        checkGLError();

        // Draw all opaque
        for (const auto &render_entity : renderable_entities) {
            Drawable *dr = world_->getComponent<Drawable>(render_entity);
            assert(dr->material != nullptr && dr->mesh != nullptr);
            Transform *tr = world_->getComponent<Transform>(render_entity);
            if (dr->material->renderPriority() == RenderPriority::Opaque) {
                renderer.render(dr->mesh.get(), dr->material.get(), tr->worldTransform());
            }
        }
        checkGLError();

        // Draw skybox if in use
        if (cam->use_skybox) {
            renderer.renderSkyBox(cam->skybox);
        }

        // Draw all transparent
        for (const auto &render_entity : renderable_entities) {
            Drawable *dr = world_->getComponent<Drawable>(render_entity);
            assert(dr->material != nullptr && dr->mesh != nullptr);
            Transform *tr = world_->getComponent<Transform>(render_entity);
            if (dr->material->renderPriority() == RenderPriority::Transparent) {
                renderer.render(dr->mesh.get(), dr->material.get(), tr->worldTransform());
            }
        }

        cam->framebuffer->done();

        // Postprocess
        renderer.resize(cam->viewport_origin.x, cam->viewport_origin.y, cam->viewport_size.x, cam->viewport_size.y);
        if (cam->postprocess.size() > 0) {
            for (int i = 0; i < cam->postprocess.size(); ++i) {
                Effect *curr_effect = cam->postprocess[i].get();
                Framebuffer *prev_fbo = (i == 0) ? cam->framebuffer.get() : cam->postprocess[i - 1]->result.get();
                curr_effect->result->use();
                prev_fbo->colorAttachment(0)->use();
                curr_effect->apply();
            }
            renderer.renderTextureToScreen(*cam->postprocess.back()->result->colorAttachment(0));
        }
        else {
            renderer.renderTextureToScreen(*cam->framebuffer->colorAttachment(0));
        }
    }
}

} // namespace rcube
