#pragma once

#include "RCube/Core/Graphics/OpenGL/Effect.h"
#include "RCube/Core/Graphics/OpenGL/Image.h"
#include "RCube/Core/Graphics/OpenGL/Light.h"
#include "RCube/Core/Graphics/OpenGL/Mesh.h"
#include "RCube/Core/Graphics/OpenGL/ShaderProgram.h"
#include "glad/glad.h"
#include "glm/glm.hpp"
#include <functional>
#include <memory>
#include <variant>
#include <vector>

namespace rcube
{

using ClearColorType = std::variant<std::monostate, int, float, glm::ivec2, glm::vec2, glm::ivec3,
                                    glm::vec3, glm::ivec4, glm::vec4>;

struct RenderTarget
{
    GLuint framebuffer;
    std::vector<ClearColorType> clear_color = {glm::vec4(1.f, 1.f, 1.f, 1.f)};
    [[deprecated]] bool clear_color_buffer = true;
    float clear_depth = 1.f;
    bool clear_depth_buffer = true;
    GLint clear_stencil = 0;
    bool clear_stencil_buffer = true;
    glm::ivec2 viewport_origin;
    glm::ivec2 viewport_size;
};

struct DrawCall
{
    // TODO(pradeep): This info structs could take in higher level
    // RCube types like Texture2D instead of OpenGL IDs...
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
    bool ignore_settings = false;
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

    void draw(const RenderTarget &render_target, const std::vector<DrawCall> &drawcalls);

    void drawTexture(const RenderTarget &render_target, std::shared_ptr<Texture2D> texture);

    void drawSkybox(const RenderTarget &render_target, std::shared_ptr<TextureCubemap> texture,
                    DrawCall dc = DrawCall{});

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

    // Skybox
    std::shared_ptr<Mesh> skybox_mesh_;
    std::shared_ptr<ShaderProgram> skybox_shader_;

    // Viewport size
    int top_, left_, width_, height_;

    // Dirty flags
    bool init_;

    // Fullscreen quad
    GLuint quad_vao_;
    std::shared_ptr<Mesh> quad_mesh_;
    std::shared_ptr<ShaderProgram> quad_shader_;
};

} // namespace rcube
