#include "Effect.h"

const std::string vs_src = R"(
#version 420
layout (location = 0) in vec3 vertex;
layout (location = 2) in vec2 texcoord;
out vec2 v_texcoord;

void main() {
    v_texcoord = texcoord;
    gl_Position = vec4(vertex, 1.0);
}
)";

Effect::Effect() : shader_(std::make_shared<ShaderProgram>()), init_(false) {
    quad_.setVertices({glm::vec3(-1.0f, 1.0f, 0.0f), glm::vec3(-1.0f, -1.0f, 0.0f),
                       glm::vec3(1.0f, 1.0f, 0.0f), glm::vec3(1.0f, -1.0f, 0.0f)});
    quad_.setTextureCoords({glm::vec2(0, 1), glm::vec2(0, 0), glm::vec2(1, 1), glm::vec2(1, 0)});
}

void Effect::initialize() {
    if (init_) {
        return;
    }
    if (!shader_->setVertexShader(vs_src, true)) {
        throw std::runtime_error("Unable to compile vertex shader");
    }
    std::string frag_src = fragmentShader();
    if (frag_src.size() > 0) {
        if (!shader_->setFragmentShader(frag_src, true)) {
            throw std::runtime_error("Unable to compile fragment shader");
        }
    }
    if (!shader_->link(true)) {
        throw std::runtime_error("Unable to link shader");
    }
    result = std::make_unique<Framebuffer>(1280, 720);
    result->initialize();
    result->addColorAttachment(TextureInternalFormat::RGBA8);
    init_ = true;
}

std::shared_ptr<ShaderProgram> Effect::shader() const {
    return shader_;
}

void Effect::resize(int width, int height) {
    result->resize(width, height);
}

void Effect::renderQuad() {
    initialize();
    glDisable(GL_DEPTH_TEST);
    quad_.use();
    shader_->use();
    setUniforms();
    shader_->drawArrays(GL_TRIANGLE_STRIP, 0, 4);
}
