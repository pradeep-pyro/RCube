#include "RCube/Core/Graphics/OpenGL/Renderer.h"
#include "RCube/Core/Graphics/OpenGL/CheckGLError.h"
#include "RCube/Core/Graphics/OpenGL/CommonMesh.h"
#include "RCube/Core/Graphics/OpenGL/CommonShader.h"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/string_cast.hpp"

namespace rcube
{

GLRenderer::GLRenderer() : top_(0), left_(0), width_(1280), height_(720), init_(false)
{
}

void GLRenderer::cleanup()
{
    if (init_)
    {
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
    // Full screen quad
    quad_mesh_ = common::fullScreenQuadMesh();
    quad_shader_ = common::fullScreenQuadShader(common::FULLSCREEN_QUAD_TEXTURE_FRAGMENT_SHADER);

    // Skybox
    skybox_mesh_ = common::skyboxMesh();
    skybox_shader_ = common::skyboxShader();

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
                static_cast<GLenum>(settings.stencil.op_stencil_pass));

    // Blending
    if (settings.blend.enabled)
    {
        glEnable(GL_BLEND);
    }
    else
    {
        glDisable(GL_BLEND);
    }
    glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
    glBlendFuncSeparate(static_cast<GLenum>(settings.blend.color_src),
                        static_cast<GLenum>(settings.blend.color_dst),
                        static_cast<GLenum>(settings.blend.alpha_src),
                        static_cast<GLenum>(settings.blend.alpha_dst));

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
    // Polygon face
    glPolygonMode(GL_FRONT_AND_BACK, static_cast<GLenum>(settings.polygon_mode));
    // Line width
    glLineWidth(settings.line_width);
    // Polygon offset
    if (settings.polygon_offset.enabled)
    {
        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonOffset(settings.polygon_offset.offset, 1.f);
    }
    else
    {
        glDisable(GL_POLYGON_OFFSET_FILL);
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
