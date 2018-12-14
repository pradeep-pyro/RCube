#ifndef BLUREFFECT_H
#define BLUREFFECT_H

#include <memory>
#include "../render/Effect.h"

/**
 * BlurEffect is a postprocessing effect that blurs the contents of the screen
 * using a 9-tap Gaussian filter
 * To use, simply add this effect to a Camera component:
 * entity.get<Camera>()->postprocess.push_back(...)
 */
class BlurEffect : public Effect {
public:
    unsigned int amount;
    BlurEffect(unsigned int amount=1);
    std::string fragmentShader() override;
    void setUniforms() override;
    void apply() override;
private:
    std::shared_ptr<Framebuffer> tmp;
};

#endif // BLUREFFECT_H
