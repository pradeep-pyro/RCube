#ifndef GAMMACORRECTION_H
#define GAMMACORRECTION_H

#include "../render/Effect.h"

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

#endif // GAMMACORRECTION_H
