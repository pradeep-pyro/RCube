#include "RCube/Core/Graphics/OpenGL/Renderer.h"
#include "RCube/Core/Graphics/Materials/FlatMaterial.h"
#include "RCube/Core/Graphics/OpenGL/CheckGLError.h"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/string_cast.hpp"
#include <iostream>

// Enable this to explicitly compute normal matrix as transpose of inverse of model matrix
// Only needed when non-uniform scaling is present
// #define RCUBE_ENABLE_NORMAL_MATRIX_COMPUTATION

namespace rcube
{

const std::vector<glm::vec3> skybox_vertices = {
    glm::vec3(-1.0f, 1.0f, -1.0f),  glm::vec3(-1.0f, -1.0f, -1.0f), glm::vec3(1.0f, -1.0f, -1.0f),
    glm::vec3(1.0f, -1.0f, -1.0f),  glm::vec3(1.0f, 1.0f, -1.0f),   glm::vec3(-1.0f, 1.0f, -1.0f),

    glm::vec3(-1.0f, -1.0f, 1.0f),  glm::vec3(-1.0f, -1.0f, -1.0f), glm::vec3(-1.0f, 1.0f, -1.0f),
    glm::vec3(-1.0f, 1.0f, -1.0f),  glm::vec3(-1.0f, 1.0f, 1.0f),   glm::vec3(-1.0f, -1.0f, 1.0f),

    glm::vec3(1.0f, -1.0f, -1.0f),  glm::vec3(1.0f, -1.0f, 1.0f),   glm::vec3(1.0f, 1.0f, 1.0f),
    glm::vec3(1.0f, 1.0f, 1.0f),    glm::vec3(1.0f, 1.0f, -1.0f),   glm::vec3(1.0f, -1.0f, -1.0f),

    glm::vec3(-1.0f, -1.0f, 1.0f),  glm::vec3(-1.0f, 1.0f, 1.0f),   glm::vec3(1.0f, 1.0f, 1.0f),
    glm::vec3(1.0f, 1.0f, 1.0f),    glm::vec3(1.0f, -1.0f, 1.0f),   glm::vec3(-1.0f, -1.0f, 1.0f),

    glm::vec3(-1.0f, 1.0f, -1.0f),  glm::vec3(1.0f, 1.0f, -1.0f),   glm::vec3(1.0f, 1.0f, 1.0f),
    glm::vec3(1.0f, 1.0f, 1.0f),    glm::vec3(-1.0f, 1.0f, 1.0f),   glm::vec3(-1.0f, 1.0f, -1.0f),

    glm::vec3(-1.0f, -1.0f, -1.0f), glm::vec3(-1.0f, -1.0f, 1.0f),  glm::vec3(1.0f, -1.0f, -1.0f),
    glm::vec3(1.0f, -1.0f, -1.0f),  glm::vec3(-1.0f, -1.0f, 1.0f),  glm::vec3(1.0f, -1.0f, 1.0f)};

const static std::string skybox_vert = R"(
#version 420

layout (location = 0) in vec3 position;
out vec3 texcoords;

layout (std140, binding=0) uniform Camera {
    mat4 view_matrix;
    mat4 projection_matrix;
    mat4 viewport_matrix;
    vec3 eye_pos;
};

void main() {
    texcoords = position;
    vec4 pos = projection_matrix * mat4(mat3(view_matrix)) * vec4(position, 1.0);
    gl_Position = pos.xyww;
}
)";

const static std::string skybox_frag = R"(
#version 420

out vec4 out_color;
in vec3 texcoords;

layout (binding = 2) uniform samplerCube skybox;

void main() {
    out_color = texture(skybox, texcoords);
}
)";

const static std::string quad_vert = R"(
#version 420
layout (location = 0) in vec3 vertex;
layout (location = 2) in vec2 texcoord;
out vec2 v_texcoord;

void main() {
    v_texcoord = texcoord;
    gl_Position = vec4(vertex, 1.0);
}
)";

const static std::string quad_frag = R"(
#version 420
in vec2 v_texcoord;
out vec4 out_color;
layout (binding=0) uniform sampler2D fbo_texture;

void main() {
   out_color = texture(fbo_texture, v_texcoord);
}
)";

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

const glm::vec3 &GLRenderer::clearColor() const
{
    return clear_color_;
}

void GLRenderer::setClearColor(const glm::vec3 &color)
{
    clear_color_ = color;
}

void GLRenderer::clear(bool color, bool depth, bool stencil)
{
    glClearColor(clear_color_[0], clear_color_[1], clear_color_[2], 1.0);
    GLbitfield clear_bits = 0;
    if (color)
    {
        clear_bits |= GL_COLOR_BUFFER_BIT;
    }
    if (depth)
    {
        clear_bits |= GL_DEPTH_BUFFER_BIT;
    }
    if (stencil)
    {
        clear_bits |= GL_STENCIL_BUFFER_BIT;
    }
    glClear(clear_bits);
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
    quad_mesh_ = Mesh::create({AttributeBuffer::create("positions", AttributeLocation::POSITION, 3),
                               AttributeBuffer::create("uvs", AttributeLocation::UV, 2)},
                              MeshPrimitive::Triangles);
    quad_mesh_->attribute("positions")
        ->setData(std::vector<glm::vec3>{glm::vec3(-1.0f, 1.0f, 0.0f),
                                         glm::vec3(-1.0f, -1.0f, 0.0f), glm::vec3(1.0f, 1.0f, 0.0f),
                                         glm::vec3(1.0f, -1.0f, 0.0f)});
    quad_mesh_->attribute("uvs")->setData(
        std::vector<glm::vec2>{glm::vec2(0, 1), glm::vec2(0, 0), glm::vec2(1, 1), glm::vec2(1, 0)});
    quad_mesh_->uploadToGPU();
    quad_shader_ = ShaderProgram::create(quad_vert, quad_frag, true);

    // Skybox
    skybox_mesh_ =
        Mesh::create({AttributeBuffer::create("positions", AttributeLocation::POSITION, 3)},
                     MeshPrimitive::Triangles);
    skybox_mesh_->attribute("positions")->setData(skybox_vertices);
    skybox_mesh_->uploadToGPU();
    skybox_shader_ = ShaderProgram::create(skybox_vert, skybox_frag, true);

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

    quad_shader_->use();
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
    if (settings.depth_test)
    {
        glEnable(GL_DEPTH_TEST);
    }
    else
    {
        glDisable(GL_DEPTH_TEST);
    }

    // Depth write
    glDepthMask(static_cast<GLboolean>(settings.depth_write));

    // Blending
    if (settings.blending)
    {
        glEnable(GL_BLEND);
        glBlendFunc(static_cast<GLenum>(settings.blendfunc_src),
                    static_cast<GLenum>(settings.blendfunc_dst));
    }
    else
    {
        glDisable(GL_BLEND);
    }

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
    if (settings.culling)
    {
        glEnable(GL_CULL_FACE);
        glCullFace(static_cast<GLenum>(settings.cull_mode));
    }
    else
    {
        glDisable(GL_CULL_FACE);
    }
}

void GLRenderer::render(Mesh *mesh, ShaderProgram *program, const glm::mat4 &model_to_world)
{
#ifdef RCUBE_ENABLE_NORMAL_MATRIX_COMPUTATION
    glm::mat3 normal_matrix = glm::mat3(glm::inverse(glm::transpose(model_to_world)));
#else
    const glm::mat3 normal_matrix(model_to_world);
#endif
    assert(program != nullptr);

    // Use shader and set uniforms
    program->use();
    Uniform u_model_matrix, u_normal_matrix;
    if (program->hasUniform("model_matrix", u_model_matrix))
    {
        u_model_matrix.set(model_to_world);
    }
    if (program->hasUniform("normal_matrix", u_normal_matrix))
    {
        u_normal_matrix.set(normal_matrix);
    }
    // mesh->use();
    glBindVertexArray(mesh->vao());
    if (mesh->numIndexData() == 0)
    {
        glDrawArrays(static_cast<GLint>(mesh->primitive()), 0,
                     static_cast<GLsizei>(mesh->numVertexData()));
    }
    else
    {
        glDrawElements(static_cast<GLint>(mesh->primitive()), (GLsizei)mesh->numIndexData(),
                       GL_UNSIGNED_INT, (void *)(0 * sizeof(uint32_t)));
    }
    glBindVertexArray(0);
}

void GLRenderer::render(Mesh *mesh, Material *material, const glm::mat4 &model_to_world)
{
#ifdef RCUBE_ENABLE_NORMAL_MATRIX_COMPUTATION
    glm::mat3 normal_matrix = glm::mat3(glm::inverse(glm::transpose(model_to_world)));
#else
    const glm::mat3 normal_matrix(model_to_world);
#endif
    assert(material != nullptr);

    // Update settings
    updateSettings(material->renderState());

    // Use shader and set uniforms
    material->use();
    Uniform u_model_matrix, u_normal_matrix;
    if (material->shader()->hasUniform("model_matrix", u_model_matrix))
    {
        u_model_matrix.set(model_to_world);
    }
    if (material->shader()->hasUniform("normal_matrix", u_normal_matrix))
    {
        u_normal_matrix.set(normal_matrix);
    }
    glBindVertexArray(mesh->vao());
    if (mesh->numIndexData() == 0)
    {
        glDrawArrays(static_cast<GLint>(mesh->primitive()), 0,
                     static_cast<GLsizei>(mesh->numVertexData()));
    }
    else
    {
        glDrawElements(static_cast<GLint>(mesh->primitive()), (GLsizei)mesh->numIndexData(),
                       GL_UNSIGNED_INT, (void *)(0 * sizeof(uint32_t)));
    }
    glBindVertexArray(0);
}

void GLRenderer::draw(const RenderTarget &render_target, const std::vector<DrawCall> &drawcalls)
{
    // TODO(pradeep): Optimize redundant state changes
    // Bind framebuffer
    render_target.framebuffer->useForWrite();
    // glBindFramebuffer(GL_DRAW_FRAMEBUFFER, render_target.framebuffer->id());
    // Clear buffers
    if (render_target.clear_color_buffer || render_target.clear_depth_buffer ||
        render_target.clear_stencil_buffer)
    {
        glClearColor(render_target.clear_color[0], render_target.clear_color[1],
                     render_target.clear_color[2], render_target.clear_color[3]);
        GLbitfield clear_bits = 0;
        if (render_target.clear_color_buffer)
        {
            clear_bits |= GL_COLOR_BUFFER_BIT;
        }
        if (render_target.clear_depth_buffer)
        {
            clear_bits |= GL_DEPTH_BUFFER_BIT;
        }
        if (render_target.clear_stencil_buffer)
        {
            clear_bits |= GL_STENCIL_BUFFER_BIT;
        }
        glClear(clear_bits);
    }

    // Draw
    for (const DrawCall& dc : drawcalls)
    {
        // Change state
        updateSettings(dc.state);
        // Bind shader
        dc.shader->use();
        // Set uniforms
        dc.update_uniforms(dc.shader);
        // Bind textures
        for (const DrawCall::DrawCallTexture2D & dctex : dc.textures)
        {
            dctex.texture->use(dctex.unit);
        }
        for (const DrawCall::DrawCallTextureCubemap &dccub : dc.cubemaps)
        {
            dccub.texture->use(dccub.unit);
        }
        // Draw
        glBindVertexArray(dc.mesh->vao());
        if (dc.mesh->numIndexData() == 0)
        {
            glDrawArrays(static_cast<GLint>(dc.mesh->primitive()), 0,
                         static_cast<GLsizei>(dc.mesh->numVertexData()));
        }
        else
        {
            glDrawElements(static_cast<GLint>(dc.mesh->primitive()),
                           (GLsizei)dc.mesh->numIndexData(),
                           GL_UNSIGNED_INT, (void *)(0 * sizeof(uint32_t)));
        }
    }
}

void GLRenderer::renderSkyBox(std::shared_ptr<TextureCubemap> cubemap)
{
    glDepthMask(GL_FALSE);
    glDepthFunc(GL_LEQUAL);
    skybox_mesh_->use();
    cubemap->use(2);
    skybox_shader_->use();
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glDepthFunc(GL_LESS);
}

void GLRenderer::renderEffect(ShaderProgram *effect, Framebuffer *input)
{
    effect->use();
    // Bind the input framebuffer's texture
    // input->colorAttachment(0)->use(0);
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    quad_mesh_->use();
    // Apply the effect on the input and render in bound framebuffer
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    effect->done();
}

void GLRenderer::renderFullscreenQuad(ShaderProgram *prog, Framebuffer *output)
{
    prog->use();
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    quad_mesh_->use();
    output->use();
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    output->done();
    prog->done();
}

void GLRenderer::renderTextureToScreen(Texture2D *tex)
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    tex->use(0);
    clear();
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    quad_mesh_->use();
    quad_shader_->use();
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

} // namespace rcube
