#pragma once

#include "RCube/Core/Graphics/Materials/Material.h"
#include "RCube/Core/Graphics/OpenGL/Effect.h"
#include "RCube/Core/Graphics/OpenGL/Image.h"
#include "RCube/Core/Graphics/OpenGL/Light.h"
#include "RCube/Core/Graphics/OpenGL/Mesh.h"
#include "RCube/Core/Graphics/OpenGL/ShaderProgram.h"
#include "glad/glad.h"
#include "glm/glm.hpp"
#include <functional>
#include <memory>
#include <vector>

namespace rcube
{

struct RenderTarget
{
    GLuint framebuffer;
    glm::vec4 clear_color = glm::vec4(1);
    bool clear_color_buffer = true;
    bool clear_depth_buffer = true;
    bool clear_stencil_buffer = true;
    glm::ivec2 viewport_origin;
    glm::ivec2 viewport_size;
};

struct DrawCall
{
    struct Texture2DInfo
    {
        GLuint texture = 0;
        int unit = 0;
    };

    struct TextureCubemapInfo
    {
        GLuint texture;
        int unit = 0;
    };

    struct MeshInfo
    {
        GLuint vao;
        GLenum primitive;
        bool indexed = false;
        GLsizei num_data;
    };

    std::shared_ptr<ShaderProgram> shader;
    std::function<void(std::shared_ptr<ShaderProgram>)> update_uniforms =
        [](std::shared_ptr<ShaderProgram>) {};
    std::vector<Texture2DInfo> textures;
    std::vector<TextureCubemapInfo> cubemaps;
    MeshInfo mesh;
    RenderSettings settings;
};

class GLRenderer
{
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
    void resize(int top, int left, size_t width, size_t height);

    void setLights(const std::vector<Light> &lights);

    void setCamera(const glm::vec3 &eye_pos, const glm::mat4 &world_to_view,
                   const glm::mat4 &view_to_projection, const glm::mat4 &projection_to_viewport);

    void render(Mesh *mesh, ShaderProgram *program, const glm::mat4 &model_to_world);

    void render(Mesh *mesh, Material *material, const glm::mat4 &model_to_world);

    void draw(const RenderTarget &render_target, const std::vector<DrawCall> &drawcalls);

    void drawTexture(const RenderTarget &render_target, std::shared_ptr<Texture2D> texture);

    void drawSkybox(const RenderTarget &render_target, std::shared_ptr<TextureCubemap> texture,
                    DrawCall dc = DrawCall{});

    void renderSkyBox(std::shared_ptr<TextureCubemap> cubemap);

    void renderTextureToScreen(std::shared_ptr<Texture2D> tex);

    void renderEffect(ShaderProgram *effect, Framebuffer *input);

    void renderFullscreenQuad(ShaderProgram *prog, Framebuffer *output);

    /**
     * @brief clearColor Returns the color that is used to clear the screen i.e.,
     * passed to glClearColor()
     * @return clear color
     */
    const glm::vec3 &clearColor() const;

    /**
     * @brief setClearColor Sets the color that is used to clear the screen i.e.,
     * passed to glClearColor()
     * @param color clear color
     */
    void setClearColor(const glm::vec3 &color);

    /**
     * @brief clear Clear the screen (calls glClear()) with set clear bits
     */
    void clear(bool color = true, bool depth = true, bool stencil = false);

    std::shared_ptr<Mesh> fullscreenQuadMesh()
    {
        return quad_mesh_;
    }

    std::shared_ptr<ShaderProgram> fullscreenQuadShader()
    {
        return quad_shader_;
    }

    std::shared_ptr<Mesh> skyboxMesh()
    {
        return skybox_mesh_;
    }

    std::shared_ptr<ShaderProgram> skyboxShader()
    {
        return skybox_shader_;
    }

    static DrawCall::MeshInfo getDrawCallMeshInfo(std::shared_ptr<Mesh> mesh);

  private:
    void updateSettings(const RenderSettings &settings);
    void updateSettings(const RenderSettings &settings, const RenderSettings &prev_settings);

    // Uniform buffer objects
    GLuint ubo_matrices_, ubo_lights_;

    // Buffer to store lights data
    std::vector<float> light_data_;

    // Skybox
    std::shared_ptr<Mesh> skybox_mesh_;
    std::shared_ptr<ShaderProgram> skybox_shader_;

    // Viewport size
    int top_, left_, width_, height_;

    // Clear settings
    glm::vec3 clear_color_;

    // Dirty flags
    bool init_;

    // Fullscreen quad
    GLuint quad_vao_;
    std::shared_ptr<Mesh> quad_mesh_;
    std::shared_ptr<ShaderProgram> quad_shader_;
};

} // namespace rcube
