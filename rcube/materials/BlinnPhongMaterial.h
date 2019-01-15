#ifndef BLINNPHONGMATERIAL_H
#define BLINNPHONGMATERIAL_H

#include <string>
#include "glm/glm.hpp"
#include "../render/Material.h"
#include "../render/Texture.h"
#include "constants.h"

namespace rcube {

/**
 * BlinnPhongMaterial is for displaying shiny surfaces with specular highlights.
 * It uses a Blinn-Phong per-pixel shading model which is not physically-based.
 */
class BlinnPhongMaterial : public Material {
public:
    BlinnPhongMaterial(glm::vec3 diffuse_color=glm::vec3(1.0), glm::vec3 specular_color=glm::vec3(0.5f), float shininess=4.f);
    BlinnPhongMaterial(const BlinnPhongMaterial &other) = default;
    BlinnPhongMaterial & operator=(const BlinnPhongMaterial &other) = default;
    virtual std::string vertexShader() override;
    virtual std::string fragmentShader() override;
    virtual std::string geometryShader() override;
    virtual void setUniforms() override;
    virtual void use() override;
    int renderPriority() const override;

    // Colors
    glm::vec3 diffuse_color;      /// Color of the diffuse material
    glm::vec3 specular_color;     /// Color of specular highlights
    float shininess;              /// Shininess is the specular exponent. Higher values result in smaller but sharper highlights
    float reflectivity;           /// Reflectivity [0.0, 1.0] controls the amount of reflection of the environment map
    // Wireframe
    bool show_wireframe;          /// Show/hide wireframe
    float wireframe_thickness;    /// Thickness of wireframe lines
    glm::vec3 wireframe_color;    /// Color of the wireframe lines
    // Textures
    std::shared_ptr<Texture2D> diffuse_texture;       /// Diffuse texture
    std::shared_ptr<Texture2D> specular_texture;      /// Specular texture
    std::shared_ptr<Texture2D> normal_texture;        /// Normal map/texture
    std::shared_ptr<TextureCubemap> environment_map;  /// Environment map texture
    Combine blend_environment_map;                    /// How to combine environment map texture with the rest
    bool use_diffuse_texture = false;   /// Whether to use diffuse texture
    bool use_specular_texture = false;  /// Whether to use specular texture
    bool use_normal_texture = false;    /// Whether to use normal texture
    bool use_environment_map = false;   /// Whether to use environment map texture
    // Rendering customizations
    bool show_backface;                 /// Whether to shade the backfacing triangles
};

} // namespace rcube

#endif // BLINNPHONGMATERIAL_H
