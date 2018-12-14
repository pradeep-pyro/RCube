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
    std::string frag_src = fragmentShader();
    shader_ = ShaderProgram::create(vs_src, frag_src ,true);
    result = Framebuffer::create(1280, 720);
    result->addColorAttachment(TextureInternalFormat::RGBA8);
    init_ = true;
}

std::shared_ptr<ShaderProgram> Effect::shader() const {
    return shader_;
}

void Effect::renderQuad() {
    initialize();
    glDisable(GL_DEPTH_TEST);
    quad_.use();
    shader_->use();
    setUniforms();
    shader_->drawArrays(GL_TRIANGLE_STRIP, 0, 4);
}
