#ifndef BLUREFFECT_H
#define BLUREFFECT_H

#include "RCube/Core/Graphics/OpenGL/Effect.h"
#include <memory>

namespace rcube
{

/**
 * BlurEffect is a postprocessing effect that blurs the contents of the screen
 * using a 9-tap Gaussian filter
 * To use, simply add this effect to a Camera component:
 * entity.get<Camera>()->postprocess.push_back(...)
 */
class BlurEffect : public Effect
{
  public:
    BlurEffect(bool horizontal = true);
    std::string fragmentShader() override;
    void setUniforms() override;

  private:
    bool horizontal_ = true;
};

} // namespace rcube

#endif // BLUREFFECT_H
