#include "GrayscaleEffect.h"

GrayscaleEffect::GrayscaleEffect() {
}

std::string GrayscaleEffect::fragmentShader() {
       return R"(
#version 420
in vec2 v_texcoord;
out vec4 out_color;
layout (binding=0) uniform sampler2D fbo_texture;

void main() {
   vec4 tex = texture(fbo_texture, v_texcoord);
   float avg = 0.2126 * tex.r + 0.7152 * tex.g + 0.0722 * tex.b;
   out_color = vec4(avg, avg, avg, tex.a);
}
       )";
}

void GrayscaleEffect::setUniforms() {
}

void GrayscaleEffect::apply() {
    result->use();
    renderQuad();
}
