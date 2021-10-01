#pragma once

#include "RCube/Components/Camera.h"
#include "RCube/Components/DirectionalLight.h"
#include "RCube/Components/Drawable.h"
#include "RCube/Components/ForwardMaterial.h"
#include "RCube/Components/Material.h"
#include "RCube/Components/PointLight.h"
#include "RCube/Components/Transform.h"
#include "RCube/Core/Arch/World.h"
#include "RCube/Core/Graphics/MeshGen/Box.h"
#include "RCube/Core/Graphics/MeshGen/Grid.h"
#include "RCube/Core/Graphics/MeshGen/Sphere.h"
#include "RCube/Core/Graphics/OpenGL/Buffer.h"
#include "RCube/Core/Graphics/OpenGL/CheckGLError.h"
#include "RCube/Core/Graphics/TexGen/CheckerBoard.h"
#include "RCube/ImGuizmo.h"
#include "RCube/Materials/DepthMaterial.h"
#include "RCube/Materials/MatCapMaterial.h"
#include "RCube/Materials/OutlineMaterial.h"
#include "RCube/Materials/StandardMaterial.h"
#include "RCube/Materials/UnlitMaterial.h"
#include "RCube/Systems/CameraSystem.h"
#include "RCube/Systems/DeferredRenderSystem.h"
#include "RCube/Systems/ForwardRenderSystem.h"
#include "RCube/Systems/TransformSystem.h"
#include "RCube/Window.h"
#include <memory>
#include <chrono>

namespace rcube
{
namespace viewer
{

struct TransformWidgetsProps
{
    bool show = false;
    float snap_angle_degrees = 0.f;
    float snap_scale = 0.f;
    glm::vec3 snap_translation = glm::vec3(0.f);
    TransformOperation operation = TransformOperation::None;
    ImGuizmo::MODE space = ImGuizmo::MODE::WORLD;
};

/**
 * A set of properties to configure the viewer
 */
struct RCubeViewerProps
{
    enum class RenderSystemType
    {
        Forward,
        Deferred
    };

    std::string title = "RCubeViewer";             // Title of the viewer window
    glm::ivec2 resolution = glm::ivec2(1280, 720); // Resolution of internal framebuffer and window
    int MSAA = 2; // Number of samples for multisampling (for RenderSystemType::Forward)
    glm::vec3 background_color_top =
        glm::vec3(82.f / 255.f, 87.f / 255.f, 110.f / 255.f); // Background top color
    glm::vec3 background_color_bottom =
        glm::vec3(82.f / 255.f, 87.f / 255.f, 110.f / 255.f); // Background bottom color
    float camera_fov = glm::radians(30.f); // Vertical FOV of the camera in radians
    [[deprecated]] bool camera_orthographic = false;
    bool ground_plane = true;           // Whether to add a ground plane grid
    bool sunlight = false;              // Whether to add a sunlight (directional light)
    bool show_transform_widgets = true; // Whether to display widgets to edit Transform components
    bool snap_transform_widgets_ = false;
    RenderSystemType render_system = RenderSystemType::Forward; // Render system to use
    TransformWidgetsProps transform_widgets;
};

class RCubeViewer : public rcube::Window
{
  protected:
    rcube::World world_;
    rcube::EntityHandle ground_;
    rcube::EntityHandle camera_;
    std::string screenshot_filename_ = "";
    std::chrono::time_point<std::chrono::high_resolution_clock> time_point_;
    bool needs_camera_extents_fit_ = false;
    bool show_transform_widgets_ = true;
    TransformWidgetsProps transform_widgets_;
    std::string selected_entity_ = "(None)";

  public:
    RCubeViewer(RCubeViewerProps props = RCubeViewerProps());

    EntityHandle addSurface(const std::string name, const TriangleMeshData &data);

    EntityHandle addEntity(const std::string &name);

    EntityHandle addMeshEntity(const std::string &name);

    EntityHandle addPointLight(const std::string name, glm::vec3 position, float radius,
                               glm::vec3 color);

    EntityHandle getEntity(std::string name);

    void removeEntity(EntityHandle ent);

    void selectEntity(const std::string &name);

    const std::string & selectedEntity() const;

    void clearSelection();

    EntityHandle camera();

    void updateImageBasedLighting();

    World &world()
    {
        return world_;
    }

    AABB worldBoundingBox();

    void fitCameraExtents();

    // Function to setup custom GUI windows
    std::function<void(RCubeViewer &viewer)> customGUI = [](RCubeViewer & /*viewer*/) {};

  protected:
    virtual void initialize() override;

    virtual void draw() override;

    virtual void drawGUI();

    virtual void drawMainMenuBarGUI();

    virtual void drawEntityInspectorGUI();

    virtual void onResize(int w, int h) override;

    virtual void beforeTerminate() override;

    glm::vec2 screenToNDC(int xpos, int ypos);

    EntityHandle createSurface();

    EntityHandle createCamera();

    EntityHandle createGroundPlane();

    EntityHandle createGroundGrid();

    EntityHandle createDirLight();

    virtual void onKeyPress(int key, int scancode) override;
};

} // namespace viewer
} // namespace rcube