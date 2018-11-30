#ifndef POINTLIGHT_H
#define POINTLIGHT_H

#include "BaseLight.h"

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
    float radius() const;
    void setRadius(float val);
};

} // namespace rcube

#endif // POINTLIGHT_H
