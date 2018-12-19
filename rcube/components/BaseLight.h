#ifndef BASELIGHT_H
#define BASELIGHT_H

#include "../render/Light.h"
#include "../ecs/component.h"

namespace rcube {

/**
 * BaseLight is the base class for all light components. It is not meant to be used directly,
 * use one of the derived classes instead: PointLight, DirectionalLight etc.
 */
class BaseLight : public Component<BaseLight> {
public:
    BaseLight() = default;
    /**
     * Returns the light data in a format understood by the renderer
     */
    const Light & light() const {
        return light_;
    }
protected:
    Light light_;  /// Struct that is understood by the renderer
};

} // namespace rcube

#endif // BASELIGHT_H
