#ifndef RENDERER_H
#define RENDERER_H

#include "glm/glm.hpp"
#include "glad/glad.h"
#include <vector>
#include <memory>
#include "Mesh.h"
#include "Material.h"
#include "Effect.h"
#include "Light.h"
#include "Image.h"
#include "skybox.h"
#include "RenderSettings.h"

class GLRenderer {
public:
    GLRenderer();

    /**
     * Initialize the renderer: uniform buffer objects (UBOs), cubemap texture etc.
     */
    void initialize();

    /**
     * cleanup Cleans any resources (e.g., GPU buffers) that the renderer might have created
     */
    void cleanup();

    /**
     * Changes the size of the viewport for rendering
     * @param top y-coordinate of top-left corner
     * @param left x-coordinate of top-left corner
     * @param width width of the viewport
     * @param height height if the viewport
     */
    void resize(int top, int left, int width, int height);

    void updateSettings(const RenderSettings &settings);

    void setLightsCamera(const std::vector<Light> &lights, const glm::mat4 &world_to_view,
                         const glm::mat4 &view_to_projection, const glm::mat4 &projection_to_viewport);

    void render(Mesh &mesh, Material *material, const glm::mat4 &model_to_world);

    void renderSkyBox(std::shared_ptr<TextureCube> cubemap);

    void renderTextureToScreen(Texture2D &tex);

    /**
     * @brief clearColor Returns the color that is used to clear the screen i.e.,
     * passed to glClearColor()
     * @return clear color
     */
    const glm::vec4 & clearColor() const;

    /**
     * @brief setClearColor Sets the color that is used to clear the screen i.e.,
     * passed to glClearColor()
     * @param color clear color
     */
    void setClearColor(const glm::vec4 &color);

    /**
     * @brief clear Clear the screen (calls glClear()) with set clear bits
     */
    void clear(bool color=true, bool depth=true, bool stencil=false);

private:

    // Uniform buffer objects
    GLuint ubo_matrices_, ubo_lights_;

    Mesh skybox_mesh_;
    ShaderProgram skybox_shader_;

    // Viewport size
    int top_, left_, width_, height_;

    // Clear settings
    glm::vec4 clear_color_;

    // Dirty flags
    bool init_;

    // Cache
    glm::mat4 world_to_view_;
    int num_lights_;

    // Quad
    Mesh quad_mesh_;
    ShaderProgram quad_shader_;
};


#endif // RENDERER_H

