#pragma once

#include "glm/glm.hpp"
#include "RCube/Core/Arch/Component.h"

namespace rcube
{

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
  public:
    // Illuminance  in lux
    // From Table 12:
    // https://google.github.io/filament/Filament.md.html#lighting/directlighting/directionallights
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
    glm::vec3 color = glm::vec3(1.f);
    glm::vec3 direction = glm::vec3(0, -1, 0);
    float intensity = float(MorningSky);
    glm::ivec2 shadowmap_origin =
        glm::ivec2(0); // Must be between (0, 0) and
                       // (RCUBE_SHADOWMAP_ATLAS_SIZE - size.x, RCUBE_SHADOWMAP_ATLAS_SIZE - size.y)
    glm::ivec2 shadowmap_size = glm::ivec2(1024); // Must be power-of-2
    bool cast_shadow = false;

    DirectionalLight() = default;

    void drawGUI();
};

} // namespace rcube
