#include "RCube/Core/Graphics/Effects/GrayscaleEffect.h"
namespace rcube
{

const static FragmentShader GRAYSCALE_EFFECT_FRAG({}, {ShaderTextureDesc{"fbo_texture", 2}}, "out_color",
                                           R"(
#version 420
in vec2 v_texcoord;
out vec4 out_color;
layout (binding=0) uniform sampler2D fbo_texture;

void main() {
   vec4 tex = texture(fbo_texture, v_texcoord);
   float avg = 0.2126 * tex.r + 0.7152 * tex.g + 0.0722 * tex.b;
   out_color = vec4(avg, avg, avg, tex.a);
}
)");

std::shared_ptr<ShaderProgram> makeGrayscaleEffect()
{
    return makeEffect(GRAYSCALE_EFFECT_FRAG);
}

} // namespace rcube
