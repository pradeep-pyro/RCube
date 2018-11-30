#include "PointLight.h"
#include <iostream>

using std::cout;
using std::endl;
namespace rcube {

PointLight::PointLight(float radius, glm::vec3 rgb) {
    cout << "PointLight constructor" << endl;
    setRadius(radius);
    light_.color = rgb;
    light_.cone_angle = glm::pi<float>();
    light_.pos_w = 1.f; // homogenous coordinate of position
}
float PointLight::radius() const {
    return light_.radius;
}
void PointLight::setRadius(float val) {
    light_.radius = val;
}

} // namespace rcube
