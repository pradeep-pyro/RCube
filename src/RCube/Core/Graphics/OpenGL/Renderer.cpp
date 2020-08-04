#include "RCube/Core/Graphics/OpenGL/Renderer.h"
#include "RCube/Core/Graphics/OpenGL/CheckGLError.h"
#include "RCube/Core/Graphics/OpenGL/CommonMesh.h"
#include "RCube/Core/Graphics/OpenGL/CommonShader.h"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/string_cast.hpp"
#include <iostream>

// Enable this to explicitly compute normal matrix as transpose of inverse of model matrix
// Only needed when non-uniform scaling is present
// #define RCUBE_ENABLE_NORMAL_MATRIX_COMPUTATION

namespace rcube
{

GLRenderer::GLRenderer()
    : top_(0), left_(0), width_(1280), height_(720), clear_color_(glm::vec4(1.f)), init_(false)
{
}

void GLRenderer::cleanup()
{
    if (init_)
    {
        glDeleteBuffers(1, &ubo_matrices_);
        glDeleteBuffers(1, &ubo_lights_);
        skybox_mesh_->release();
        skybox_shader_->release();
        quad_mesh_->release();
        quad_shader_->release();
        init_ = false;
    }
}

void GLRenderer::initialize()
{
    if (init_)
    {
        return;
    }
    glGenBuffers(1, &ubo_matrices_);
    glBindBuffer(GL_UNIFORM_BUFFER, ubo_matrices_);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::mat4) * 3 + sizeof(glm::vec3), nullptr,
                 GL_DYNAMIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    glGenBuffers(1, &ubo_lights_);
    glBindBuffer(GL_UNIFORM_BUFFER, ubo_lights_);
    // 99 lights with 12 floats and 1 int for num_lights and 3 ints since GLSL std140 expects
    // 16-byte alignment
    glBufferData(GL_UNIFORM_BUFFER, sizeof(float) * 12 * 99 + 1 * sizeof(int), nullptr,
                 GL_DYNAMIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    // Full screen quad
    quad_mesh_ = common::fullScreenQuadMesh();
    quad_shader_ = common::fullScreenQuadShader(common::FULLSCREEN_QUAD_TEXTURE_FRAGMENT_SHADER);

    // Skybox
    skybox_mesh_ = common::skyboxMesh();
    skybox_shader_ = common::skyboxShader();

    // Lights data
    light_data_.reserve(99 * 12);
    init_ = true;
}

void GLRenderer::resize(int top, int left, size_t width, size_t height)
{
    top_ = top;
    left_ = left;
    width_ = (int)width;
    height_ = (int)height;

    glEnable(GL_SCISSOR_TEST);
    glViewport(top_, left_, static_cast<GLsizei>(width_), static_cast<GLsizei>(height_));
    glScissor(top_, left_, static_cast<GLsizei>(width_), static_cast<GLsizei>(height_));
}

void GLRenderer::setCamera(const glm::vec3 &eye_pos, const glm::mat4 &world_to_view,
                           const glm::mat4 &view_to_projection,
                           const glm::mat4 &projection_to_viewport)
{
    initialize();

    // const glm::vec3 eye_pos = glm::vec3(glm::inverse(world_to_view)[3]);

    const int float4x4_size = sizeof(glm::mat4);
    glBindBuffer(GL_UNIFORM_BUFFER, ubo_matrices_);
    // Copy view, projection and viewport matrices to UBO
    glBufferSubData(GL_UNIFORM_BUFFER, 0, float4x4_size, glm::value_ptr(world_to_view));
    glBufferSubData(GL_UNIFORM_BUFFER, float4x4_size, float4x4_size,
                    glm::value_ptr(view_to_projection));
    glBufferSubData(GL_UNIFORM_BUFFER, 2 * float4x4_size, float4x4_size,
                    glm::value_ptr(projection_to_viewport));
    // Copy eye pos to UBO
    glBufferSubData(GL_UNIFORM_BUFFER, 3 * float4x4_size, static_cast<int>(sizeof(glm::vec3)),
                    glm::value_ptr(eye_pos));
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo_matrices_);
}

void GLRenderer::setLights(const std::vector<Light> &lights)
{
    initialize();
    // Copy lights
    assert(lights.size() < 99);
    light_data_.clear();
    for (const Light &l : lights)
    {
        const glm::vec3 &pos_xyz = l.position;
        // pos_xyz = glm::vec3(world_to_view * glm::vec4(pos_xyz, 1.f));
        const glm::vec3 &dir = l.direction;
        // dir = glm::vec3(world_to_view * glm::vec4(dir, 0.f));
        light_data_.push_back(pos_xyz.x);
        light_data_.push_back(pos_xyz.y);
        light_data_.push_back(pos_xyz.z);
        light_data_.push_back(l.pos_w);
        light_data_.push_back(dir.x);
        light_data_.push_back(dir.y);
        light_data_.push_back(dir.z);
        light_data_.push_back(l.radius);
        light_data_.push_back(l.color.r);
        light_data_.push_back(l.color.g);
        light_data_.push_back(l.color.b);
        light_data_.push_back(l.cone_angle);
    }
    const int num_lights = static_cast<int>(lights.size());
    glBindBuffer(GL_UNIFORM_BUFFER, ubo_lights_);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, light_data_.size() * sizeof(float), light_data_.data());
    glBufferSubData(GL_UNIFORM_BUFFER, 99 * 12 * sizeof(float), sizeof(int), &num_lights);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    glBindBufferBase(GL_UNIFORM_BUFFER, 2, ubo_lights_);
}

void GLRenderer::updateSettings(const RenderSettings &settings)
{
    // Depth test
    if (settings.depth.test)
    {
        glEnable(GL_DEPTH_TEST);
    }
    else
    {
        glDisable(GL_DEPTH_TEST);
    }
    glDepthMask(static_cast<GLboolean>(settings.depth.write));
    glDepthFunc(static_cast<GLenum>(settings.depth.func));

    // Stencil test
    if (settings.stencil.test)
    {
        glEnable(GL_STENCIL_TEST);
    }
    else
    {
        glDisable(GL_STENCIL_TEST);
    }
    glStencilMask(settings.stencil.write);
    glStencilFunc(static_cast<GLenum>(settings.stencil.func),
                  static_cast<GLenum>(settings.stencil.func_ref),
                  static_cast<GLenum>(settings.stencil.func_mask));
    glStencilOp(static_cast<GLenum>(settings.stencil.op_stencil_fail),
                static_cast<GLenum>(settings.stencil.op_depth_fail),
                static_cast<GLenum>(settings.stencil.op_pass));

    // Blending
    if (settings.blend.enabled)
    {
        glEnable(GL_BLEND);
    }
    else
    {
        glDisable(GL_BLEND);
    }
    glBlendFunc(static_cast<GLenum>(settings.blend.func_src),
                static_cast<GLenum>(settings.blend.func_dst));

    // Dithering
    if (settings.dither)
    {
        glEnable(GL_DITHER);
    }
    else
    {
        glDisable(GL_DITHER);
    }
    // Face Culling
    glCullFace(static_cast<GLenum>(settings.cull.mode));
    if (settings.cull.enabled)
    {
        glEnable(GL_CULL_FACE);
    }
    else
    {
        glDisable(GL_CULL_FACE);
    }
}

void GLRenderer::draw(const RenderTarget &render_target, const std::vector<DrawCall> &drawcalls)
{
    // TODO(pradeep): Optimize redundant state changes
    // Bind framebuffer
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, render_target.framebuffer);
    resize(render_target.viewport_origin[0], render_target.viewport_origin[1],
           render_target.viewport_size[0], render_target.viewport_size[1]);

    // Enable writing in all buffers for clearing state
    // Clear buffers
    if (render_target.clear_color_buffer)
    {
        glClearColor(render_target.clear_color[0], render_target.clear_color[1],
                     render_target.clear_color[2], render_target.clear_color[3]);
    }
    GLbitfield clear_bits = 0;
    if (render_target.clear_color_buffer)
    {
        clear_bits |= GL_COLOR_BUFFER_BIT;
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    }
    if (render_target.clear_depth_buffer)
    {
        clear_bits |= GL_DEPTH_BUFFER_BIT;
        glDepthMask(GL_TRUE);
    }
    if (render_target.clear_stencil_buffer)
    {
        clear_bits |= GL_STENCIL_BUFFER_BIT;
        glStencilMask(0xFF);
    }
    glClear(clear_bits);

    // Draw
    for (const DrawCall &dc : drawcalls)
    {
        // Change state
        updateSettings(dc.settings);
        // Bind shader
        dc.shader->use();
        // Set uniforms
        dc.update_uniforms(dc.shader);
        // Bind textures
        for (const DrawCall::Texture2DInfo &dctex : dc.textures)
        {
            glBindTextureUnit(dctex.unit, dctex.texture);
        }
        for (const DrawCall::TextureCubemapInfo &dccub : dc.cubemaps)
        {
            glBindTextureUnit(dccub.unit, dccub.texture);
        }
        // Draw
        glBindVertexArray(dc.mesh.vao);
        if (!dc.mesh.indexed)
        {
            glDrawArrays(dc.mesh.primitive, 0, dc.mesh.num_data);
        }
        else
        {
            glDrawElements(dc.mesh.primitive, dc.mesh.num_data, GL_UNSIGNED_INT,
                           (void *)(0 * sizeof(uint32_t)));
        }
    }
    glDisable(GL_SCISSOR_TEST);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

void GLRenderer::drawTexture(const RenderTarget &render_target, std::shared_ptr<Texture2D> texture)
{
    DrawCall dc;
    dc.settings.depth.test = false;
    dc.settings.depth.write = true;
    dc.shader = quad_shader_;
    dc.textures.push_back({texture->id(), 0});
    dc.mesh = getDrawCallMeshInfo(quad_mesh_);
    dc.settings.depth.test = false;
    dc.settings.depth.write = true;
    draw(render_target, {dc});
}

void GLRenderer::drawSkybox(const RenderTarget &render_target,
                            std::shared_ptr<TextureCubemap> texture, DrawCall dc)
{
    dc.settings.depth.write = false;
    dc.settings.depth.test = true;
    dc.settings.depth.func = DepthFunc::LessOrEqual;
    dc.cubemaps.push_back({texture->id(), 0});
    dc.shader = skybox_shader_;
    dc.mesh = getDrawCallMeshInfo(skybox_mesh_);
    draw(render_target, {dc});
}

DrawCall::MeshInfo GLRenderer::getDrawCallMeshInfo(std::shared_ptr<Mesh> mesh)
{
    DrawCall::MeshInfo mesh_info;
    mesh_info.indexed = mesh->numIndexData() > 0;
    mesh_info.num_data = GLsizei(mesh_info.indexed ? mesh->numIndexData() : mesh->numVertexData());
    mesh_info.primitive = static_cast<GLenum>(mesh->primitive());
    mesh_info.vao = mesh->vao();
    return mesh_info;
}

} // namespace rcube
