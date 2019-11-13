#ifndef GAMMACORRECTIONEFFECT_H
#define GAMMACORRECTIONEFFECT_H

#include "RCube/Core/Graphics/OpenGL/Effect.h"
namespace rcube {

/**
 * GammaCorrectionEffect is a postprocessing effect that converts the contents of the screen
 * from RGB to grayscale while leaving the alpha channel untouched.
 * To use, simply add this effect to a Camera component:
 * entity.get<Camera>()->postprocess.push_back(...)
 */
class GammaCorrectionEffect : public Effect {
public:
    GammaCorrectionEffect() = default;
    std::string fragmentShader() override;
    void setUniforms() override;
};

} // namespace rcube

#endif // GAMMACORRECTIONEFFECT_H
