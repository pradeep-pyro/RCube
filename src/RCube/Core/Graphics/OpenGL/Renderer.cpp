#include "RCube/Core/Graphics/OpenGL/Renderer.h"
#include "RCube/Core/Graphics/Materials/FlatMaterial.h"
#include "RCube/Core/Graphics/OpenGL/CheckGLError.h"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/string_cast.hpp"

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

const static VertexShader skybox_vert({ShaderAttributeDesc("position", GLDataType::Vec3f)}, {}, R"(
#version 420

layout (location = 0) in vec3 position;
out vec3 texcoords;

layout (std140, binding=0) uniform Matrices {
    mat4 view_matrix;
    mat4 projection_matrix;
    mat4 viewport_matrix;
};

void main() {
    texcoords = position;
    vec4 pos = projection_matrix * mat4(mat3(view_matrix)) * vec4(position, 1.0);
    gl_Position = pos.xyww;
}
)");

const static FragmentShader skybox_frag({}, {}, {ShaderCubemapDesc{"skybox"}}, "out_color",
                                        R"(
#version 420

out vec4 out_color;
in vec3 texcoords;

layout(binding = 3) uniform samplerCube skybox;

void main() {
    out_color = texture(skybox, texcoords);
}
)");

const static VertexShader quad_vert({ShaderAttributeDesc("vertex", GLDataType::Vec3f),
                                     ShaderAttributeDesc("texcoord", GLDataType::Vec2f)},
                                    {}, R"(
#version 420
layout (location = 0) in vec3 vertex;
layout (location = 2) in vec2 texcoord;
out vec2 v_texcoord;

void main() {
    v_texcoord = texcoord;
    gl_Position = vec4(vertex, 1.0);
}
)");

const static FragmentShader quad_frag({}, {ShaderTextureDesc{"fbo_texture", 2}}, {}, "out_color",
                                      R"(
#version 420
in vec2 v_texcoord;
out vec4 out_color;
layout (binding=0) uniform sampler2D fbo_texture;

void main() {
   out_color = texture(fbo_texture, v_texcoord);
}
)");

GLRenderer::GLRenderer()
    : top_(0), left_(0), width_(1280), height_(720), clear_color_(glm::vec4(1.f)), init_(false),
      num_lights_(0)
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

const glm::vec4 &GLRenderer::clearColor() const
{
    return clear_color_;
}

void GLRenderer::setClearColor(const glm::vec4 &color)
{
    clear_color_ = color;
}

void GLRenderer::clear(bool color, bool depth, bool stencil)
{
    glClearColor(clear_color_[0], clear_color_[1], clear_color_[2], clear_color_[3]);
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
    glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::mat4) * 3, nullptr, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    glGenBuffers(1, &ubo_lights_);
    glBindBuffer(GL_UNIFORM_BUFFER, ubo_lights_);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(float) * 12 * 99, nullptr, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    // Full screen quad
    quad_mesh_ = Mesh::create();
    quad_mesh_->data.vertices = {glm::vec3(-1.0f, 1.0f, 0.0f), glm::vec3(-1.0f, -1.0f, 0.0f),
                                 glm::vec3(1.0f, 1.0f, 0.0f), glm::vec3(1.0f, -1.0f, 0.0f)};
    quad_mesh_->data.texcoords = {glm::vec2(0, 1), glm::vec2(0, 0), glm::vec2(1, 1),
                                  glm::vec2(1, 0)};
    quad_mesh_->uploadToGPU();
    quad_shader_ = ShaderProgram::create(quad_vert, quad_frag, true);

    // Skybox
    skybox_mesh_ = Mesh::create();
    skybox_mesh_->data.vertices = skybox_vertices;
    skybox_mesh_->uploadToGPU();
    skybox_shader_ = ShaderProgram::create(skybox_vert, skybox_frag, true);

    init_ = true;
}

void GLRenderer::resize(int top, int left, int width, int height)
{
    top_ = top;
    left_ = left;
    width_ = width;
    height_ = height;

    glEnable(GL_SCISSOR_TEST);
    glViewport(top_, left_, width_, height_);
    glScissor(top_, left_, width_, height_);

    quad_shader_->use();
}

void GLRenderer::setCamera(const glm::mat4 &world_to_view, const glm::mat4 &view_to_projection,
                           const glm::mat4 &projection_to_viewport)
{
    initialize();
    // Copy projection and viewport matrices to UBO
    int float4x4_size = sizeof(glm::mat4);
    glBindBuffer(GL_UNIFORM_BUFFER, ubo_matrices_);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, float4x4_size, glm::value_ptr(world_to_view));
    glBufferSubData(GL_UNIFORM_BUFFER, float4x4_size, float4x4_size,
                    glm::value_ptr(view_to_projection));
    glBufferSubData(GL_UNIFORM_BUFFER, 2 * float4x4_size, float4x4_size,
                    glm::value_ptr(projection_to_viewport));
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo_matrices_);
    eye_pos_ = glm::vec3(glm::inverse(world_to_view)[3]);
}

void GLRenderer::setLights(const std::vector<Light> &lights)
{
    initialize();
    // Copy lights
    std::vector<float> light_data;
    assert(lights.size() < 99);
    light_data.reserve(lights.size() * 12);
    for (const Light &l : lights)
    {
        glm::vec3 pos_xyz = l.position;
        // pos_xyz = glm::vec3(world_to_view * glm::vec4(pos_xyz, 1.f));
        glm::vec3 dir = glm::vec3(l.direction);
        // dir = glm::vec3(world_to_view * glm::vec4(dir, 0.f));
        light_data.push_back(pos_xyz.x);
        light_data.push_back(pos_xyz.y);
        light_data.push_back(pos_xyz.z);
        light_data.push_back(l.pos_w);
        light_data.push_back(dir.x);
        light_data.push_back(dir.y);
        light_data.push_back(dir.z);
        light_data.push_back(l.radius);
        light_data.push_back(l.color.r);
        light_data.push_back(l.color.g);
        light_data.push_back(l.color.b);
        light_data.push_back(l.cone_angle);
    }
    num_lights_ = lights.size();
    glBindBuffer(GL_UNIFORM_BUFFER, ubo_lights_);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, light_data.size() * sizeof(float), light_data.data());
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
    glm::mat3 normal_matrix = glm::mat3(glm::inverse(glm::transpose(model_to_world)));

    assert(program != nullptr);

    // Update settings
    updateSettings(program->renderState());

    // Use shader and set uniforms
    program->use();
    try
    {
        program->uniform("model_matrix").set(model_to_world);
        program->uniform("eye_pos").set(eye_pos_);
        program->uniform("normal_matrix").set(normal_matrix);
        program->uniform("num_lights").set(static_cast<int>(num_lights_));
    }
    catch (const std::exception)
    {
    }
    mesh->use();
    if (!mesh->indexed())
    {
        program->drawArrays(static_cast<GLint>(mesh->data.primitive), 0, mesh->numVertices());
    }
    else
    {
        program->drawElements(static_cast<GLint>(mesh->data.primitive), 0, mesh->numPrimitives());
    }
}

void GLRenderer::renderSkyBox(std::shared_ptr<TextureCubemap> cubemap)
{
    glDepthMask(GL_FALSE);
    glDepthFunc(GL_LEQUAL);
    cubemap->use(3);
    skybox_shader_->use();
    skybox_mesh_->use();
    skybox_shader_->drawArrays(GL_TRIANGLES, 0, 36);
    glDepthFunc(GL_LESS);
}


void GLRenderer::renderEffect(ShaderProgram *effect, Framebuffer *input)
{
    effect->use();
    // Bind the input framebuffer's texture
    input->colorAttachment(0)->use(0);
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    quad_mesh_->use();
    // Apply the effect on the input and render in bound framebuffer
    effect->drawArrays(GL_TRIANGLE_STRIP, 0, 4);
    effect->done();
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
    quad_shader_->drawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

} // namespace rcube
