#ifndef PHYSICALLYBASEDMATERIAL_H
#define PHYSICALLYBASEDMATERIAL_H

#include "../render/Material.h"

class PhysicallyBasedMaterial : public Material {
public:
    PhysicallyBasedMaterial(glm::vec3 color=glm::vec3(1.0), float metalness=0.5f, float roughness=0.4f);
    PhysicallyBasedMaterial(const PhysicallyBasedMaterial &other) = default;
    PhysicallyBasedMaterial & operator=(const PhysicallyBasedMaterial &other) = default;
    virtual std::string vertexShader() override;
    virtual std::string fragmentShader() override;
    virtual std::string geometryShader() override;
    virtual void setUniforms() override;
    virtual void use() override;
    int renderPriority() const override;
};

#endif // PHYSICALLYBASEDMATERIAL_H
