#ifndef FLATMATERIAL_H
#define FLATMATERIAL_H

#include "constants.h"
#include "RCube/Core/Graphics/OpenGL/Material.h"

namespace rcube {

/**
 * FlatMaterial is for representing objects in a flat style without any 3D shading
 */
class FlatMaterial: public Material {
public:
    FlatMaterial();
    std::string vertexShader() override;
    std::string fragmentShader() override;
    std::string geometryShader() override;
    void setUniforms() override;
    int renderPriority() const override;
};

} // namespace rcube

#endif // FLATMATERIAL_H
