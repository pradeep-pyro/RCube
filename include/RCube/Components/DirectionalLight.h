#pragma once

#include "RCube/Core/Arch/Component.h"
#include "RCube/Core/Graphics/OpenGL/Texture.h"
#include "glm/glm.hpp"

#define RCUBE_MAX_DIRECTIONAL_LIGHTS 5

namespace rcube
{

/** Illuminance  in lux
 * From Table 12:
 * https://google.github.io/filament/Filament.md.html#lighting/directlighting/directionallights
 * TODO: Use Physically based units throughout
 */
enum Illuminance
{
    MorningSun = 100'000,
    MiddaySun = 105'000,
    EveningSun = 90'000,
    MorningSky = 20'000,
    MiddaySky = 25'000,
    EveningSky = 9'000,
    Moon = 1,
};

/**
 * The DirectionalLight class represents an infinite distance light source that is not
 * attenuated.
 * To create a valid directional light, add a DirectionalLight component to an Entity.
 *
 * The direction is taken to be the xyz coordinates given by the position in the Transform
 * component.
 */
class DirectionalLight : public Component<DirectionalLight>
{
    glm::vec3 color_ = glm::vec3(1.f);
    glm::vec3 direction_ = glm::vec3(0, -1, 0);
    float intensity_ = 1.f;
    bool cast_shadow_ = false;

  public:
    DirectionalLight() = default;

    /**
     * Get the direction of the light
     * @return Direction
     */
    const glm::vec3 &direction() const;

    /**
     * Set the direction of the light
     * @param dir Direction
     */
    void setDirection(const glm::vec3 &dir);

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

    glm::mat4 viewProjectionMatrix(float left=-5.f, float right=5.f, float bottom=-5.f, float top=5.f, float znear=0.f, float zfar=50.f) const;

    void drawGUI();
};

} // namespace rcube
