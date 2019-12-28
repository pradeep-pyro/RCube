#include "RCubeViewer/RCubeViewer.h"

namespace rcube
{
namespace viewer
{

RCubeViewer::RCubeViewer(RCubeViewerProps props) : Window(props.title)
{
    world_.addSystem(std::make_unique<TransformSystem>());
    world_.addSystem(std::make_unique<CameraSystem>());
    auto rs = std::make_unique<RenderSystem>();
    world_.addSystem(std::move(rs));

    // Create a default camera
    camera_ = createCamera();
    camera_.get<Transform>()->setPosition(props.camera_position);
    camera_.get<Camera>()->fov = props.camera_fov;
    camera_.get<Camera>()->background_color = props.background_color;

    // Create a directional light along view direction
    {
        EntityHandle light = createDirLight();
        light.get<Transform>()->setParent(camera_.get<Transform>());
    }
    
    // Create a ground plane
    ground_ = createGroundPlane();

    ctrl_.resize(props.resolution.x, props.resolution.y);
    ctrl_.setEntity(camera_);
    world_.initialize();
}

EntityHandle RCubeViewer::addIcoSphereSurface(const std::string name, float radius,
                                              int numSubdivisions)
{
    EntityHandle ent = createSurface();
    ent.add<Name>(name);

    std::shared_ptr<Mesh> sphereMesh = Mesh::create();
    std::shared_ptr<BlinnPhongMaterial> blinnPhong =
        std::make_shared<BlinnPhongMaterial>(default_surface_color_);
    sphereMesh->data = icoSphere(radius, numSubdivisions);
    sphereMesh->uploadToGPU();
    ent.get<Drawable>()->mesh = sphereMesh;
    ent.get<Drawable>()->material = blinnPhong;
    return ent;
}

EntityHandle RCubeViewer::addCubeSphereSurface(const std::string name, float radius,
                                               int numSegments)
{
    EntityHandle ent = createSurface();
    ent.add<Name>(name);

    std::shared_ptr<Mesh> sphereMesh = Mesh::create();
    std::shared_ptr<BlinnPhongMaterial> blinnPhong =
        std::make_shared<BlinnPhongMaterial>(default_surface_color_);
    sphereMesh->data = cubeSphere(radius, numSegments);
    sphereMesh->uploadToGPU();
    ent.get<Drawable>()->mesh = sphereMesh;
    ent.get<Drawable>()->material = blinnPhong;
    return ent;
}

EntityHandle RCubeViewer::addBoxSurface(const std::string name, float width, float height,
                                        float depth, int width_segments, int height_segments,
                                        int depth_segments, int numSegments)
{
    EntityHandle ent = createSurface();
    ent.add<Name>(name);

    std::shared_ptr<Mesh> boxMesh = Mesh::create();
    std::shared_ptr<BlinnPhongMaterial> blinnPhong =
        std::make_shared<BlinnPhongMaterial>(default_surface_color_);
    boxMesh->data = box(width, height, depth, width_segments, height_segments, depth_segments);
    boxMesh->uploadToGPU();
    ent.get<Drawable>()->mesh = boxMesh;
    ent.get<Drawable>()->material = blinnPhong;
    return ent;
}

EntityHandle RCubeViewer::addSurface(const std::string name, const MeshData &data)
{
    EntityHandle ent = createSurface();
    ent.add<Name>(name);

    std::shared_ptr<Mesh> mesh = Mesh::create();
    std::shared_ptr<BlinnPhongMaterial> blinnPhong =
        std::make_shared<BlinnPhongMaterial>(default_surface_color_);
    mesh->data = data;
    mesh->uploadToGPU();
    ent.get<Drawable>()->mesh = mesh;
    ent.get<Drawable>()->material = blinnPhong;
    return ent;
}

EntityHandle RCubeViewer::getEntity(std::string name)
{
    auto it = world_.entities();
    while (it.hasNext())
    {
        EntityHandle ent = it.next();
        Name *name_component = nullptr;
        try
        {
            name_component = ent.get<Name>();
        }
        catch (const std::exception & /*e*/)
        {
        }
        if (name_component != nullptr && name_component->name == name)
        {
            return ent;
        }
    }
    return EntityHandle();
}

EntityHandle RCubeViewer::camera()
{
    return camera_;
}

void RCubeViewer::draw()
{
    world_.update();
}

void RCubeViewer::onResize(int w, int h)
{
    ctrl_.resize(w, h);
    camera_.get<rcube::Camera>()->viewport_size = glm::ivec2(w, h);
}

void RCubeViewer::onScroll(double xoffset, double yoffset)
{
    ctrl_.zoom(yoffset);
}

void RCubeViewer::beforeTerminate()
{
    world_.cleanup();
}

void RCubeViewer::onMousePress(int key, int mods)
{
    glm::dvec2 pos = getMousePosition();
    if (key == GLFW_MOUSE_BUTTON_MIDDLE)
    {
        ctrl_.startPanning(pos.x, pos.y);
    }
    else if (key == GLFW_MOUSE_BUTTON_RIGHT)
    {
        ctrl_.startOrbiting(pos.x, pos.y);
    }
}
void RCubeViewer::onMouseRelease(int key, int mods)
{
    glm::dvec2 pos = getMousePosition();
    if (key == GLFW_MOUSE_BUTTON_MIDDLE)
    {
        ctrl_.stopPanning(pos.x, pos.y);
    }
    else if (key == GLFW_MOUSE_BUTTON_RIGHT)
    {
        ctrl_.stopOrbiting(pos.x, pos.y);
    }
}
void RCubeViewer::onMouseMove(double xpos, double ypos)
{
    glm::dvec2 pos = getMousePosition();
    ctrl_.orbit(pos.x, pos.y);
    ctrl_.pan(pos.x, pos.y);
}

///////////////////////////////////////////////////////////////////////////////

EntityHandle RCubeViewer::createSurface()
{
    auto ent = world_.createEntity();
    auto dr = Drawable();
    ent.add<Drawable>(dr);
    ent.add<Transform>(Transform());
    return ent;
}

EntityHandle RCubeViewer::createCamera()
{
    auto ent = world_.createEntity();
    ent.add<Camera>(Camera());
    ent.add<Transform>(Transform());
    return ent;
}

EntityHandle RCubeViewer::createGroundPlane()
{
    std::shared_ptr<Mesh> gridMesh = Mesh::create();
    gridMesh->data = rcube::grid(20, 20, 100, 100, glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0),
                                 glm::vec3(0.0, 0.0, 0.0));
    gridMesh->uploadToGPU();
    std::shared_ptr<FlatMaterial> flat = std::make_shared<FlatMaterial>();
    ground_ = createSurface();
    ground_.get<Drawable>()->mesh = gridMesh;
    ground_.get<Drawable>()->material = flat;
    return ground_;
}

EntityHandle RCubeViewer::createDirLight()
{
    EntityHandle ent = world_.createEntity();
    ent.add(DirectionalLight());
    ent.add(Transform());
    return ent;
}

} // namespace viewer
} // namespace rcube