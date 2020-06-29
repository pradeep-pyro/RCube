#ifndef CAMERA_H
#define CAMERA_H

#include "RCube/Core/Arch/Component.h"
#include "RCube/Core/Graphics/OpenGL/Effect.h"
#include "RCube/Core/Graphics/OpenGL/Texture.h"
#include "glm/glm.hpp"
#include <array>
#include <vector>

namespace rcube
{

class CameraSystem;
class RenderSystem;

constexpr glm::vec3 XAXIS_POSITIVE = glm::vec3(+1, 0, 0);
constexpr glm::vec3 XAXIS_NEGATIVE = glm::vec3(-1, 0, 0);
constexpr glm::vec3 YAXIS_POSITIVE = glm::vec3(0, +1, 0);
constexpr glm::vec3 YAXIS_NEGATIVE = glm::vec3(0, -1, 0);
constexpr glm::vec3 ZAXIS_POSITIVE = glm::vec3(0, 0, +1);
constexpr glm::vec3 ZAXIS_NEGATIVE = glm::vec3(0, 0, -1);

struct Frustum
{
    std::array<glm::vec3, 8> points;
};

/**
 * Camera is the component to display the world on the screen.
 * To create a valid camera object, add a Camera component (camera's characteristics) and
 * Transform component (camera's location) to an Entity.
 */
class Camera : public Component<Camera>
{
  public:
    Camera()
        : world_to_view(glm::mat4(1)), view_to_projection(glm::mat4(1)),
          projection_to_viewport(glm::mat4(1))
    {
    }
    bool orthographic = false;      /// Whether the camera uses orthographic projection
    float fov = glm::radians(60.f); /// Field of view when using perspective projection
    float near_plane = 0.01f; /// The closest point relative to the camera which will be be drawn
    float far_plane = 1000.f; /// The farthest point relative to the camera which will be be drawn
    float orthographic_size =
        2; /// Used to control field of view indirectly when using orthographic projection
    glm::vec3 target = glm::vec3(0.f, 0.f, 0.f);
    bool rendering =
        true;                                   /// Whether the camera is actively rendering
    glm::ivec2 viewport_origin = glm::ivec2(0); /// Origin of the viewport where the scene is drawn
    glm::ivec2 viewport_size =
        glm::ivec2(1280, 720); /// Size of the viewport where the scene is drawn
    glm::vec4 background_color =
        glm::vec4(1); /// Background color for the scene when viewed from this camera
    std::shared_ptr<TextureCubemap> skybox; /// Skybox texture
    bool use_skybox = false;                /// Whether to draw a skybox
    std::vector<std::shared_ptr<ShaderProgram>>
        postprocess; /// Postprocessing effects applied to the scene in order

    /**
     * Computes and returns the frustum representing the camera's view
     * @return View frustum
     */
    Frustum frustum();

    const glm::mat4 &worldToView() const
    {
        return world_to_view;
    }

    const glm::mat4 &viewToProjection() const
    {
        return view_to_projection;
    }

    const glm::mat4 &projectionToViewport() const
    {
        return projection_to_viewport;
    }

    void drawGUI();

  private:
    friend class CameraSystem;        // This will update the camera matrices
    friend class RenderSystem;        // This will make use of the matrices
    glm::mat4 world_to_view;          /// World to camera transformation
    glm::mat4 view_to_projection;     /// Camera to projection transformation
    glm::mat4 projection_to_viewport; /// Projectin to viewport transformation
};

} // namespace rcube

#endif // CAMERA_H
