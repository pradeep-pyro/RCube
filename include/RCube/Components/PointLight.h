#ifndef POINTLIGHT_H
#define POINTLIGHT_H

#include "RCube/Components/BaseLight.h"

namespace rcube {

/**
 * PointLight represents a light that emits light in all directions.
 * Its intensity diminishes with distance based on the inverse-square law.
 * To create a valid point light, add a PointLight component and a
 * Transform component (light's position) to an entity.
 */
class PointLight : public BaseLight {
public:
    PointLight(float radius=1.f, glm::vec3 color=glm::vec3(1.f));
    /**
     * Get the radius of the point light
     * @return Radius
     */
    float radius() const;
    /**
     * Set the radius of the point light which controls its area of influence
     * @param val Radius
     */
    void setRadius(float val);

    /**
     * Get the color of the light
     * @return Color
     */
    const glm::vec3 & color() const;

    /**
     * Set the color of the light
     * @param rgb Color
     */
    void setColor(const glm::vec3 &rgb);
};

} // namespace rcube

#endif // POINTLIGHT_H
