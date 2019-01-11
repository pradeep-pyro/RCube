#ifndef BLUREFFECT_H
#define BLUREFFECT_H

#include <memory>
#include "../render/Effect.h"

namespace rcube {

/**
 * BlurEffect is a postprocessing effect that blurs the contents of the screen
 * using a 9-tap Gaussian filter
 * To use, simply add this effect to a Camera component:
 * entity.get<Camera>()->postprocess.push_back(...)
 */
class BlurEffect : public Effect {
public:
    unsigned int amount;  /// Number of times blur (each iteration corresponds to 9x9 guassian blur)

    BlurEffect(unsigned int amount=1);
    std::string fragmentShader() override;
    void setUniforms() override;
    void use() override;
    void done() override;
private:
    std::shared_ptr<Framebuffer> tmp;
};

} // namespace rcube

#endif // BLUREFFECT_H
