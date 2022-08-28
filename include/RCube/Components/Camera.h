#pragma once

#include "RCube/Core/Arch/Component.h"
#include "RCube/Core/Graphics/OpenGL/Effect.h"
#include "RCube/Core/Graphics/OpenGL/Texture.h"
#include "glm/glm.hpp"
#include <array>
#include <vector>
#include "imgui.h"
#include "RCube/ImGuizmo.h"

namespace rcube
{

class CameraSystem;
class RenderSystem;
class DeferredRenderSystem;

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
    bool rendering = true;                      /// Whether the camera is actively rendering
    glm::ivec2 viewport_origin = glm::ivec2(0); /// Origin of the viewport where the scene is drawn
    glm::ivec2 viewport_size =
        glm::ivec2(1280, 720); /// Size of the viewport where the scene is drawn
    std::shared_ptr<TextureCubemap> skybox; /// Skybox texture
    bool use_skybox = false;                /// Whether to draw a skybox
    std::shared_ptr<TextureCubemap> irradiance;
    std::shared_ptr<TextureCubemap> prefilter;
    std::shared_ptr<Texture2D> brdfLUT;
    float bloom_threshold = 1000.f;
    
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

    void fitToExtents(const AABB &box_in_world_space);

    void createGradientSkyBox(const glm::vec3 &color_top, const glm::vec3 &color_bot);

    void drawGUI();

    glm::vec3 screenToWorld(glm::vec2 xy, float distance_from_camera);

  private:
    friend class CameraSystem;         // This will update the camera matrices
    friend class RenderSystem;         // This will make use of the matrices
    friend class DeferredRenderSystem; // This will make use of the matrices
    friend class ForwardRenderSystem;
    glm::mat4 world_to_view;           /// World to camera transformation
    glm::mat4 view_to_projection;      /// Camera to projection transformation
    glm::mat4 projection_to_viewport;  /// Projection to viewport transformation
    AABB fit_to_box_;
    bool needs_fit_to_extents_ = false;
};

} // namespace rcube
