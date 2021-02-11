#pragma once

#include <string>

namespace rcube
{
namespace shaders
{

const static std::string BrightnessFragmentShader = R"(
#version 450
in vec2 v_texcoord;
out vec4 out_color;
layout (binding=0) uniform sampler2D fbo_texture;
uniform float bloom_threshold;

float luminance(const vec3 rgb)
{
    return dot(rgb, vec3(0.2126, 0.7152, 0.0722));
}

void main()
{
    vec3 pixel = texture(fbo_texture, v_texcoord).rgb;
    vec3 result = (luminance(pixel) > bloom_threshold) ? pixel : vec3(0, 0, 0);
    out_color = vec4(result, 1.0);
}
)";

const static std::string BlurFragmentShader = R"(
#version 450
in vec2 v_texcoord;
out vec4 out_color;
layout (binding=0) uniform sampler2D fbo_texture;

uniform bool horizontal;
uniform float weight[5] = float[](0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);

// https://learnopengl.com/Advanced-Lighting/Bloom
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

const static std::string PostprocessFragmentShader = R"(
#version 450
in vec2 v_texcoord;
out vec4 out_color;
layout (binding=0) uniform sampler2D scene;
layout (binding=1) uniform sampler2D blur;

vec3 tonemapReinhard(const vec3 color)
{
    return color / (color + vec3(1.0));
}

void main() {
    // Bloom https://learnopengl.com/Advanced-Lighting/Bloom
    vec3 hdr_color = texture(scene, v_texcoord).rgb;
    vec3 bloom_color = texture(blur, v_texcoord).rgb;
    vec3 result = hdr_color + bloom_color;
    // Tone mapping
    result = tonemapReinhard(result);
    // Gamma correction
    const float gamma = 2.2;
    result = pow(result, vec3(1.0 / gamma));
    out_color = vec4(result, 1.0);
}
)";

} // namespace shaders
} // namespace rcube