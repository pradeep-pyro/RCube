#ifndef PHYSICALLYBASEDMATERIAL_H
#define PHYSICALLYBASEDMATERIAL_H

#include <string>
#include "glm/glm.hpp"
#include "../render/Material.h"
#include "../render/Texture.h"
#include "constants.h"

namespace rcube {

/**
 * PhysicallyBasedMaterial is for displaying realistic surfaces with varying roughness and metalness characteristics.
 * It uses a physically-based Cook-Torrance BRDF for computing specular.
 */
class PhysicallyBasedMaterial : public Material {
public:
    PhysicallyBasedMaterial(glm::vec3 albedo=glm::vec3(1.0), float roughness=0.5f, float metalness=0.5f);
    PhysicallyBasedMaterial(const PhysicallyBasedMaterial &other) = default;
    PhysicallyBasedMaterial & operator=(const PhysicallyBasedMaterial &other) = default;
    virtual std::string vertexShader() override;
    virtual std::string fragmentShader() override;
    virtual std::string geometryShader() override;
    virtual void setUniforms() override;
    virtual void use() override;
    int renderPriority() const override;

    // Colors
    glm::vec3 albedo;      /// True color of the material under while light without shadows/AO
    float roughness;              /// How rough is the surface [0.0, 1.0], where 0.0 is smooth
    float metalness;               /// How metallic is the surface [0.0, 1.0], where 0.0 is dielectric
    // Wireframe
    bool show_wireframe;          /// Show/hide wireframe
    float wireframe_thickness;    /// Thickness of wireframe lines
    glm::vec3 wireframe_color;    /// Color of the wireframe lines
    // Textures
    std::shared_ptr<Texture2D> albedo_texture;       /// Diffuse texture
    std::shared_ptr<Texture2D> roughness_texture;     /// Roughness texture
    std::shared_ptr<Texture2D> metalness_texture;     /// Roughness texture
    std::shared_ptr<Texture2D> normal_texture;        /// Normal map/texture
    std::shared_ptr<TextureCubemap> environment_map;  /// Environment map texture
    Combine blend_environment_map;                    /// How to combine environment map texture with the rest
    bool use_albedo_texture = false;    /// Whether to use diffuse texture
    bool use_roughness_texture = false;  /// Whether to use roughness texture
    bool use_metalness_texture = false;   /// Whether to use metalness texture
    bool use_normal_texture = false;     /// Whether to use normal texture
    bool use_environment_map = false;    /// Whether to use environment map texture
};

} // namespace rcube

#endif // PHYSICALLYBASEDMATERIAL_H
