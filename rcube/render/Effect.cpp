#include "Effect.h"

namespace rcube {

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
    result = Framebuffer::create(1280, 720);
    result->addColorAttachment(TextureInternalFormat::RGBA8);
}

void Effect::initialize() {
    if (init_) {
        return;
    }
    std::string frag_src = fragmentShader();
    shader_ = ShaderProgram::create(vs_src, frag_src ,true);
    init_ = true;
}

std::shared_ptr<ShaderProgram> Effect::shader() const {
    return shader_;
}

void Effect::use() {
    initialize();
    shader_->use();
    result->use();
    setUniforms();
}

void Effect::done() {
    // Nothing to do by default
}

} // namespace rcube
