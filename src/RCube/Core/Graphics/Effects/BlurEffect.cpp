#include "RCube/Core/Graphics/Effects/BlurEffect.h"

namespace rcube
{

const static FragmentShader BLUR_EFFECT_FRAG({ShaderUniformDesc{"horizonal", GLDataType::Bool}},
                                      "out_color", R"(
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
)");



std::shared_ptr<ShaderProgram> makeBlurEffect(bool horizonal)
{
    auto prog = makeEffect(BLUR_EFFECT_FRAG);
    prog->uniform("horizonal").set(horizonal);
    return prog;
}

} // namespace rcube
