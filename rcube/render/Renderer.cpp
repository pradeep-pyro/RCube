#include "Renderer.h"
#include "glm/gtx/string_cast.hpp"
#include "glm/gtc/matrix_transform.hpp"

const std::string quad_vert_src = R"(
#version 420
layout (location = 0) in vec3 vertex;
layout (location = 2) in vec2 texcoord;
out vec2 v_texcoord;

uniform mat4 projection;

void main() {
    v_texcoord = texcoord;
    gl_Position = vec4(vertex, 1.0);
}
)";

const std::string quad_frag_src = R"(
#version 420
in vec2 v_texcoord;
out vec4 out_color;
layout (binding=0) uniform sampler2D fbo_texture;

void main() {
   out_color = texture(fbo_texture, v_texcoord);
}
)";

GLRenderer::GLRenderer()
    : top_(0), left_(0), width_(1280), height_(720),
      clear_color_(glm::vec4(1.f)), init_(false),  world_to_view_(glm::mat4(1)), num_lights_(0) {
}

void GLRenderer::cleanup() {
    if (init_) {
        glDeleteBuffers(1, &ubo_matrices_);
        glDeleteBuffers(1, &ubo_lights_);
        skybox_.free();
        quad_mesh_.release();
        quad_shader_.release();
        init_ = false;
    }
}

const glm::vec4 & GLRenderer::clearColor() const {
    return clear_color_;
}

void GLRenderer::setClearColor(const glm::vec4 &color) {
    clear_color_ = color;
}

void GLRenderer::clear(bool color, bool depth, bool stencil) {
    glClearColor(clear_color_[0], clear_color_[1], clear_color_[2], clear_color_[3]);
    GLbitfield clear_bits = 0;
    if (color) {
        clear_bits |= GL_COLOR_BUFFER_BIT;
    }
    if (depth) {
        clear_bits |= GL_DEPTH_BUFFER_BIT;
    }
    if (stencil) {
        clear_bits |= GL_STENCIL_BUFFER_BIT;
    }
    glClear(clear_bits);
}

void GLRenderer::initialize() {
    if (init_) {
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

    quad_mesh_.initialize();
    quad_mesh_.setVertices({glm::vec3(-1.0f, 1.0f, 0.0f), glm::vec3(-1.0f, -1.0f, 0.0f),
                       glm::vec3(1.0f, 1.0f, 0.0f), glm::vec3(1.0f, -1.0f, 0.0f)});
    quad_mesh_.setTextureCoords({glm::vec2(0, 1), glm::vec2(0, 0), glm::vec2(1, 1), glm::vec2(1, 0)});
    quad_shader_.setVertexShader(quad_vert_src);
    quad_shader_.setFragmentShader(quad_frag_src);
    quad_shader_.link();
    quad_shader_.use();
    quad_shader_.setUniform("projection", glm::mat4(1));

    glEnable(GL_SCISSOR_TEST);

    init_ = true;
}

void GLRenderer::resize(int top, int left, int width, int height) {
    top_ = top;
    left_ = left;
    width_ = width;
    height_ = height;

    glViewport(top_, left_, width_, height_);
    glScissor(top_, left_, width_, height_);

    quad_shader_.use();
    GLfloat aspectRatio = static_cast<float>(width) / static_cast<float>(height);
    glm::mat4 mat_projection = glm::ortho(-aspectRatio, aspectRatio, -1.f, 1.f);
    quad_shader_.setUniform("projection", mat_projection);
}

void GLRenderer::setLightsCamera(const std::vector<Light> &lights, const glm::mat4 &world_to_view,
                                 const glm::mat4 &view_to_projection, const glm::mat4 &projection_to_viewport) {
    initialize();
    // Copy projection and viewport matrices to UBO
    int float4x4_size = sizeof(glm::mat4);
    glBindBuffer(GL_UNIFORM_BUFFER, ubo_matrices_);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, float4x4_size, glm::value_ptr(world_to_view));
    glBufferSubData(GL_UNIFORM_BUFFER, float4x4_size, float4x4_size, glm::value_ptr(view_to_projection));
    glBufferSubData(GL_UNIFORM_BUFFER, 2*float4x4_size, float4x4_size, glm::value_ptr(projection_to_viewport));
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo_matrices_);

    // Store view matrix to multiply with model matrix later
    world_to_view_ = world_to_view;

    // Copy lights
    std::vector<float> light_data;
    assert (lights.size() < 99);
    light_data.reserve(lights.size() * 12);
    for (const Light &l : lights) {
        glm::vec3 pos_xyz = l.position;
        pos_xyz = glm::vec3(world_to_view * glm::vec4(pos_xyz, 1.f));
        glm::vec3 dir = glm::vec3(l.direction);
        dir = glm::vec3(world_to_view * glm::vec4(dir, 0.f));
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

void GLRenderer::render(Mesh &mesh, Material *material, const glm::mat4 &model_to_world) {
    glm::mat4 model_view = world_to_view_ * model_to_world;
    glm::mat3 normal_matrix = glm::mat3(glm::inverse(glm::transpose(model_view)));

    assert (material != nullptr);
    material->use();
    std::shared_ptr<ShaderProgram> sh = material->shader();
    sh->setUniform("modelview_matrix", model_view);
    sh->setUniform("normal_matrix",normal_matrix);
    sh->setUniform("num_lights", static_cast<int>(num_lights_));

    //  Draw
    mesh.use();
    if (!mesh.indexed()) {
        material->shader()->drawArrays(static_cast<GLint>(mesh.primitive()),
                                                0, mesh.numVertices());
    }
    else {
        material->shader()->drawElements(static_cast<GLint>(mesh.primitive()),
                                                  0, mesh.numPrimitives());
    }
}

void GLRenderer::renderSkyBox(std::shared_ptr<TextureCube> cubemap) {
    skybox_.texture = cubemap;
    skybox_.render();
}

void GLRenderer::renderTextureToScreen(Texture2D &tex) {
    tex.use(0);
    glDisable(GL_DEPTH_TEST);
    quad_mesh_.use();
    quad_shader_.use();
    //clear();
    quad_shader_.drawArrays(GL_TRIANGLE_STRIP, 0, 4);
}
