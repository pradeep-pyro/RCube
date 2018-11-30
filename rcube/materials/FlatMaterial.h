#ifndef FLATCOLOR_H
#define FLATCOLOR_H

#include "constants.h"
#include "../render/Material.h"

class FlatMaterial: public Material {
public:
    FlatMaterial();
    std::string vertexShader() override;
    std::string fragmentShader() override;
    std::string geometryShader() override;
    void setUniforms() override;
    int renderPriority() const override;
};

#endif // FLATCOLOR_H
