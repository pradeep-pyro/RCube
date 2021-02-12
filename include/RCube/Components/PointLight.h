#pragma once

#include "RCube/Core/Arch/Component.h"
#include "glm/glm.hpp"

#define RCUBE_MAX_POINT_LIGHTS 100

namespace rcube
{

/**
 * PointLight represents a light that emits light in all directions.
 * Its intensity diminishes with distance based on the inverse-square law.
 * To create a valid point light, add a PointLight component and a
 * Transform component (light's position) to an entity.
 */
class PointLight : public Component<PointLight>
{
    bool cast_shadow_ = false;
    float intensity_ = 1.f;
    glm::vec3 position_ = glm::vec3(0, 0, 0);
    float radius_ = 1.f;
    glm::vec3 color_ = glm::vec3(1, 1, 1);

  public:
    PointLight(float radius = 1.f, glm::vec3 color = glm::vec3(1.f));

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
    const glm::vec3 &color() const;

    /**
     * Set the color of the light
     * @param rgb Color
     */
    void setColor(const glm::vec3 &rgb);

    /**
     * Whether this light casts a shadow
     * @return Cast shadow or not
     */
    bool castShadow() const;

    /**
     * Sets whether this light casts a shadow
     * @param Cast shadow or not
     */
    void setCastShadow(bool val);

    /**
     * Get the intensity of the light
     * @return Intensity
     */
    float intensity() const;

    /**
     * Get the intensity of the light
     * @return Intensity
     */
    void setIntensity(float val);

    /**
     * Draw and control this component's data using ImGui
     */
    void drawGUI();
};

} // namespace rcube
