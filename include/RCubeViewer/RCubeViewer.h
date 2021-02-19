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
#include "RCube/Materials/DepthMaterial.h"
#include "RCube/Materials/StandardMaterial.h"
#include "RCube/Materials/MatCapMaterial.h"
#include "RCube/Materials/UnlitMaterial.h"
#include "RCube/Systems/CameraSystem.h"
#include "RCube/Systems/DeferredRenderSystem.h"
#include "RCube/Systems/ForwardRenderSystem.h"
#include "RCube/Systems/TransformSystem.h"
#include "RCube/Window.h"
#include <memory>

namespace rcube
{
namespace viewer
{

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
    int MSAA = 4;                                  // Number of samples for multisampling
    glm::vec3 background_color_top =
        glm::vec3(82.f / 255.f, 87.f / 255.f, 110.f / 255.f); // Background top color
    glm::vec3 background_color_bottom =
        glm::vec3(82.f / 255.f, 87.f / 255.f, 110.f / 255.f); // Background bottom color
    float camera_fov = glm::radians(30.f); // Vertical FOV of the camera in radians
    [[deprecated]] bool camera_orthographic = false;
    bool ground_plane = true; // Whether to add a ground plane grid
    bool sunlight = true;     // Whether to add a sunlight (directional light)
    RenderSystemType render_system = RenderSystemType::Forward; // Render system to use
};

class RCubeViewer : public rcube::Window
{
  protected:
    rcube::World world_;
    rcube::EntityHandle ground_;
    rcube::EntityHandle camera_;

    glm::vec3 default_surface_color_ = glm::vec3(0.75, 0.75, 0.75);
    std::shared_ptr<ShaderMaterial> default_shader_;

  public:
    RCubeViewer(RCubeViewerProps props = RCubeViewerProps());

    EntityHandle addSurface(const std::string name, const TriangleMeshData &data);

    EntityHandle addEntity(const std::string &name);

    EntityHandle addMeshEntity(const std::string &name);

    EntityHandle addPointLight(const std::string name, glm::vec3 position, float radius,
                               glm::vec3 color);

    EntityHandle getEntity(std::string name);

    void removeEntity(EntityHandle ent);

    EntityHandle camera();

    void updateImageBasedLighting();

    World &world()
    {
        return world_;
    }

    // Function to setup custom GUI windows
    std::function<void(RCubeViewer &viewer)> customGUI = [](RCubeViewer & /*viewer*/) {};

  protected:
    virtual void initialize() override;

    virtual void draw() override;

    virtual void drawGUI();

    virtual void onResize(int w, int h) override;

    virtual void beforeTerminate() override;

    glm::vec2 screenToNDC(int xpos, int ypos);

    EntityHandle createSurface();

    EntityHandle createCamera();

    EntityHandle createGroundPlane();

    EntityHandle createGroundGrid();

    EntityHandle createDirLight();
};

} // namespace viewer
} // namespace rcube