#ifndef LIGHT_H
#define LIGHT_H

#include "glm/glm.hpp"
#include "glm/gtc/constants.hpp"

namespace rcube {

/**
 * Light stores data for all kinds of lights that can be sent
 * to the shaders.
 * This is used internally; users can work with the specific light classes instead.
 */
struct Light {
    Light() : position(glm::vec3(0)), pos_w(1.f), radius(1.f), direction(glm::vec3(0, 0, 1)),
              cone_angle(glm::half_pi<float>()), color(glm::vec3(1)), ambient(glm::vec3(0.2f)) {
    }
    glm::vec3 position;
    float pos_w; // set to 0 for directional light
    float radius;
    glm::vec3 direction;
    float cone_angle;
    glm::vec3 color;
    glm::vec3 ambient;
};

} // namespace rcube

#endif // LIGHT_H
