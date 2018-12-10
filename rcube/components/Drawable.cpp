#include "Drawable.h"
#include "../materials/FlatMaterial.h"

namespace rcube {

//Drawable::Drawable() : mesh(std::make_shared<Mesh>()), material(std::make_shared<FlatMaterial>()), visible(true) {
Drawable::Drawable() : mesh(Mesh()), material(std::make_shared<FlatMaterial>()), visible(true) {
}

} // namespace rcube
