#include "BlurEffect.h"

BlurEffect::BlurEffect(unsigned int amount)
    : amount(amount), tmp(std::make_unique<Framebuffer>(1280, 720, TextureInternalFormat::RGBA8)) {
}

void BlurEffect::setUniforms() {
}

std::string BlurEffect::fragmentShader() {
    return R"(
#version 420
in vec2 v_texcoord;
out vec4 out_color;
layout (binding=0) uniform sampler2D fbo_texture;

uniform bool horizontal;
uniform float weight[5] = float[](0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);

void main() {
    vec3 result = weight[0] * texture(fbo_texture, v_texcoord).rgb;
    vec2 texel_size = 1.0 / vec2(textureSize(fbo_texture, 0));
    if (horizontal) {
        for (int i = 1; i < 5; ++i) {
            result += weight[i] * texture(fbo_texture, v_texcoord + vec2(texel_size.x * float(i), 0)).rgb;
            result += weight[i] * texture(fbo_texture, v_texcoord - vec2(texel_size.x * float(i), 0)).rgb;
        }
    }
    else {
        for (int i = 1; i < 5; ++i) {
            result += weight[i] * texture(fbo_texture, v_texcoord + vec2(0, texel_size.x * float(i))).rgb;
            result += weight[i] * texture(fbo_texture, v_texcoord - vec2(0, texel_size.x * float(i))).rgb;
        }
    }
    out_color = vec4(result, 1.0);
}
    )";
}

void BlurEffect::apply() {
    shader_->use();
    for (unsigned int i = 0; i < amount * 2; ++i) {
        if (i % 2 == 0) {
            tmp->use();
            shader_->setUniform("horizontal", true);
            if (i > 0) {
                result->colorAttachment(0)->use();
            }
        }
        else {
            result->use();
            shader_->setUniform("horizontal", false);
            if (i > 0) {
                tmp->colorAttachment(0)->use();
            }
        }
        renderQuad();
    }
    if (amount % 2 == 1) {
        tmp->blit(*result);
    }
}

void BlurEffect::resize(int width, int height) {
    Effect::resize(width, height);
    tmp->resize(width, height);
}
