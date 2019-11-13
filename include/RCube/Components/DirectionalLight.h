#ifndef DIRECTIONLIGHT_H
#define DIRECTIONLIGHT_H

#include "RCube/Components/BaseLight.h"

namespace rcube {

/**
 * The DirectionalLight class represents an infinite distance light source that is not
 * attenuated.
 * To create a valid directional light, add a DirectionalLight component (camera's characteristics) and a
 * Transform component (light's location) to an Entity.
 *
 * The direction is taken to be the xyz coordinates given by the position in the Transform component.
 */
class DirectionalLight : public BaseLight {
public:
    DirectionalLight(const glm::vec3 &color=glm::vec3(1.f));
    /**
     * Set the color of the light
     * @param rgb Color
     */
    void setColor(const glm::vec3 &rgb);

    /**
     * Get the color of the light
     * @return Color
     */
    const glm::vec3 & color() const;
};

} // namespace rcube

#endif // DIRECTIONLIGHT_H
