#ifndef CAMERA_H
#define CAMERA_H

#include "../ecs/component.h"
#include "glm/glm.hpp"
#include "../render/Texture.h"
#include "../render/Effect.h"
#include <vector>

namespace rcube {

class CameraSystem;
class RenderSystem;

/**
 * Camera is the component to display the world on the screen.
 * To create a valid camera object, add a Camera component (camera's characteristics) and
 * Transform component (camera's location) to an Entity.
 */
class Camera : public Component<Camera> {
public:
    Camera() : skybox(std::make_shared<TextureCube>()), use_skybox(false),
        //framebuffer(std::make_shared<Framebuffer>(1280, 720, TextureInternalFormat::RGBA8, TextureInternalFormat::Depth24Stencil8)),
        world_to_view(glm::mat4(1)), view_to_projection(glm::mat4(1)), projection_to_viewport(glm::mat4(1)) {
    }
    bool orthographic = false;          /// Whether the camera uses orthographic projection
    float fov = glm::radians(60.f);     /// Field of view when using perspective projection
    float near = 0.1f;                  /// The closest point relative to the camera which will be be drawn.
    float far = 300.f;                  /// The farthest point relative to the camera which will be be drawn.
    float orthographic_size = 2;        /// Used to control field of view indirectly when using orthographic projection
    glm::vec3 target = glm::vec3(0);    /// Target where the camera points to
    glm::vec3 up = glm::vec3(0, 1, 0);  /// Up orientation w.r.t. the camera
    bool rendering = true;              /// Whether the camera is actively rendering
    glm::ivec2 viewport_origin = glm::ivec2(0);        /// Origin of the viewport where the scene is drawn
    glm::ivec2 viewport_size = glm::ivec2(1280, 720);  /// Size of the viewport where the scene is drawn
    glm::vec4 background_color = glm::vec4(1);         /// Background color for the scene when viewed from this camera
    std::shared_ptr<TextureCube> skybox; /// Skybox texture
    bool use_skybox = false;                     /// Whether to draw a skybox
    std::shared_ptr<Framebuffer> framebuffer;
    std::vector<std::shared_ptr<Effect>> postprocess; /// Postprocessing effects applied to the scene in order
    void resize(int width, int height) {
        viewport_size.x = width;
        viewport_size.y = height;
    }
private:
    void initFBO() {
        if (framebuffer != nullptr) {
            return;
        }
        framebuffer = Framebuffer::create(1280, 720);
        framebuffer->addColorAttachment(TextureInternalFormat::RGBA8);
        framebuffer->addDepthAttachment(TextureInternalFormat::Depth24Stencil8);
    }
    friend class CameraSystem; // This will update the camera matrices
    friend class RenderSystem; // This will make use of the matrices
    glm::mat4 world_to_view, view_to_projection, projection_to_viewport; // Should these be public?
};

} // namespace rcube

#endif // CAMERA_H
