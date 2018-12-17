#ifndef GRAYSCALEEFFECT_H
#define GRAYSCALEEFFECT_H

#include "../render/Effect.h"

/**
 * GrayscaleEffect is a postprocessing effect that converts the contents of the screen
 * from RGB to grayscale while leaving the alpha channel untouched.
 * To use, simply add this effect to a Camera component:
 * entity.get<Camera>()->postprocess.push_back(...)
 */
class GrayscaleEffect : public Effect {
public:
    GrayscaleEffect();
    std::string fragmentShader() override;
    void setUniforms() override;
};

#endif // GRAYSCALEEFFECT_H
