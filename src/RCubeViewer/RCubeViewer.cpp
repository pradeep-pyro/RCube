#include "RCubeViewer/RCubeViewer.h"
#include "RCube/Core/Arch/World.h"
#include "RCube/Core/Graphics/ImageBasedLighting/IBLDiffuse.h"
#include "RCube/Core/Graphics/ImageBasedLighting/IBLSpecularSplitSum.h"
#include "RCube/Core/Graphics/MeshGen/Plane.h"
#include "RCube/Core/Graphics/TexGen/CheckerBoard.h"
#include "RCube/Core/Graphics/TexGen/Gradient.h"
#include "RCube/Systems/DeferredRenderSystem.h"
#include "RCubeViewer/Components/CameraController.h"
#include "RCubeViewer/Components/Name.h"
#include "RCubeViewer/Systems/CameraControllerSystem.h"
#include "RCubeViewer/Systems/PickSystem.h"
#include "RCubeViewer/Systems/PickTooltipSystem.h"
#include "glm/gtx/euler_angles.hpp"
#include "glm/gtx/string_cast.hpp"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

namespace rcube
{
namespace viewer
{

void initImGUI(GLFWwindow *window)
{

    ImGui::CreateContext();

    // Set up ImGUI glfw bindings
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    const char *glsl_version = "#version 420";
    ImGui_ImplOpenGL3_Init(glsl_version);

    ImGuiIO &io = ImGui::GetIO();
    ImFontConfig config;
    config.OversampleH = 5;
    config.OversampleV = 5;

    ImGui::StyleColorsClassic();
}

RCubeViewer::RCubeViewer(RCubeViewerProps props) : Window(props.title)
{
    world_.addSystem(std::make_unique<TransformSystem>());
    world_.addSystem(std::make_unique<CameraSystem>());
    if (props.render_system == RCubeViewerProps::RenderSystemType::Deferred)
    {
        world_.addSystem(std::make_unique<DeferredRenderSystem>(props.resolution));
    }
    else
    {
        world_.addSystem(std::make_unique<ForwardRenderSystem>(props.resolution, props.MSAA));
    }
    world_.addSystem(std::make_unique<CameraControllerSystem>());
    world_.addSystem(std::make_unique<PickSystem>());
    world_.addSystem(std::make_unique<PickTooltipSystem>());

    // Create a default camera
    camera_ = createCamera();
    camera_.get<Transform>()->lookAt(glm::vec3(0.f, 0.f, 1.5f), glm::vec3(0.f, 0.f, 0.f),
                                     YAXIS_POSITIVE);
    camera_.get<Camera>()->fov = props.camera_fov;
    // Pradeep: Disabling orthographic camera since it doen't play well with skyboxes
    // camera_.get<Camera>()->orthographic = props.camera_orthographic;
    // Make a default skybox
    camera_.get<Camera>()->createGradientSkyBox(props.background_color_top,
                                                props.background_color_bottom);
    camera_.get<Camera>()->use_skybox = true;

    // Create a sunlight
    if (props.sunlight)
    {
        auto dirl = createDirLight();
        dirl.add(Name("SunLight"));
        dirl.get<DirectionalLight>()->setIntensity(20.f);
        dirl.get<DirectionalLight>()->setCastShadow(true);
    }

    // Create a ground plane
    if (props.ground_plane)
    {
        if (props.render_system == RCubeViewerProps::RenderSystemType::Deferred)
        {
            // Deferred renderer cannot handle different material/geometry types
            ground_ = createGroundPlane();
        }
        else
        {
            ground_ = createGroundGrid();
        }
    }

    world_.initialize();

    IMGUI_CHECKVERSION();
    initImGUI(window_);
}

EntityHandle RCubeViewer::addSurface(const std::string name, const TriangleMeshData &data)
{
    EntityHandle ent = createSurface();
    ent.add<Name>(name);

    std::shared_ptr<Mesh> mesh = Mesh::create(data);
    mesh->uploadToGPU();
    ent.get<Drawable>()->mesh = mesh;
    return ent;
}

EntityHandle RCubeViewer::addEntity(const std::string &name)
{
    auto ent = world_.createEntity();
    ent.add<Name>(name);
    return ent;
}

EntityHandle RCubeViewer::addMeshEntity(const std::string &name)
{
    auto ent = world_.createEntity();
    ent.add<Drawable>(Drawable());
    if (world_.getSystem("ForwardRenderSystem") != nullptr)
    {
        ent.add<ForwardMaterial>();
    }
    else if (world_.getSystem("DeferredRenderSystem") != nullptr)
    {
        ent.add<Material>();
    }
    ent.add<Transform>(Transform());
    ent.add<Name>(name);
    return ent;
}

EntityHandle RCubeViewer::addPointLight(const std::string name, glm::vec3 position, float radius,
                                        glm::vec3 color)
{
    EntityHandle ent = world_.createEntity();
    ent.add(Name(name));
    ent.add(PointLight(radius, color));
    Transform tr;
    tr.setPosition(position);
    ent.add(tr);
    return ent;
}

EntityHandle RCubeViewer::getEntity(std::string name)
{
    auto it = world_.entities();
    while (it.hasNext())
    {
        EntityHandle ent = it.next();
        Name *name_component = nullptr;
        if (ent.has<Name>())
        {
            name_component = ent.get<Name>();
        }
        if (name_component != nullptr && name_component->name == name)
        {
            return ent;
        }
    }
    return EntityHandle();
}

void RCubeViewer::removeEntity(EntityHandle ent)
{
    world_.removeEntity(ent);
}

EntityHandle RCubeViewer::camera()
{
    return camera_;
}

void RCubeViewer::initialize()
{
    // Update the PBR image-based lighting maps for all object's materials
    updateImageBasedLighting();
}

void RCubeViewer::draw()
{
    // Initialize ImGui
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // Create GUI
    drawGUI();
    customGUI(*this);

    // Render everything in the scene
    world_.update();
    ImGui::Render();

    // Draw GUI
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void RCubeViewer::drawGUI()
{
    ImGui::Begin("RCubeViewer");

    ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate,
                ImGui::GetIO().Framerate);

    // Screenshot
    static ImVec4 coral_color = ImVec4(1.f, 127.f / 255.f, 80.f / 255.f, 1.f);
    ImGui::PushStyleColor(ImGuiCol_Button, coral_color);
    if (ImGui::Button("Screenshot", ImVec2(ImGui::GetContentRegionAvailWidth(), 0.f)))
    {
        needs_screenshot_ = true;
    }
    ImGui::PopStyleColor();

    ///////////////////////////////////////////////////////////////////////////
    // Default camera
    if (ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_DefaultOpen))
    {
        Camera *cam = camera_.get<Camera>();
        Transform *cam_tr = camera_.get<Transform>();
        CameraController *cam_ctrl = camera_.get<CameraController>();

        // Fit camera to extents if requested in previous frame
        if (needs_camera_extents_fit_)
        {
            fitCameraExtents();
            needs_camera_extents_fit_ = false;
        }

        // Default camera viewpoints
        if (ImGui::Button("+X"))
        {
            cam->target = glm::vec3(0.f);
            cam_tr->lookAt(glm::vec3(+1.5f, 0, 0), glm::vec3(0.f), YAXIS_POSITIVE);
            needs_camera_extents_fit_ = true;
        }
        ImGui::SameLine();
        if (ImGui::Button("-X"))
        {
            cam->target = glm::vec3(0.f);
            cam_tr->lookAt(glm::vec3(-1.5f, 0, 0), glm::vec3(0.f), YAXIS_POSITIVE);
            needs_camera_extents_fit_ = true;
        }
        ImGui::SameLine();
        if (ImGui::Button("+Y"))
        {
            cam->target = glm::vec3(0.f);
            cam_tr->lookAt(glm::vec3(0, +1.5f, 0), glm::vec3(0.f), ZAXIS_POSITIVE);
            needs_camera_extents_fit_ = true;
        }
        ImGui::SameLine();
        if (ImGui::Button("-Y"))
        {
            cam->target = glm::vec3(0.f);
            cam_tr->lookAt(glm::vec3(0, -1.5f, 0), glm::vec3(0.f), ZAXIS_POSITIVE);
            needs_camera_extents_fit_ = true;
        }
        ImGui::SameLine();
        if (ImGui::Button("+Z"))
        {
            cam->target = glm::vec3(0.f);
            cam_tr->lookAt(glm::vec3(0, 0, +1.5f), glm::vec3(0.f), YAXIS_POSITIVE);
            needs_camera_extents_fit_ = true;
        }
        ImGui::SameLine();
        if (ImGui::Button("-Z"))
        {
            cam->target = glm::vec3(0.f);
            cam_tr->lookAt(glm::vec3(0, 0, -1.5f), glm::vec3(0.f), YAXIS_POSITIVE);
            needs_camera_extents_fit_ = true;
        }
        if (ImGui::Button("Fit extents"))
        {
            needs_camera_extents_fit_ = true;
        }
        cam->drawGUI();
        cam_ctrl->drawGUI();
    }

    ///////////////////////////////////////////////////////////////////////////

    if (ImGui::CollapsingHeader("Entities", ImGuiTreeNodeFlags_DefaultOpen))
    {
        auto it = world_.entities();

        std::vector<const char *> entity_names;
        entity_names.reserve(world_.numEntities() + 1);
        entity_names.push_back("(None)");
        while (it.hasNext())
        {
            EntityHandle ent = it.next();
            if (ent.entity == camera_.entity)
            {
                continue;
            }
            entity_names.push_back(ent.get<Name>()->name.c_str());
        }

        static const char *current_item = entity_names[0];
        if (ImGui::BeginCombo("", current_item))
        {
            for (int i = 0; i < entity_names.size(); ++i)
            {
                bool is_selected = (current_item == entity_names[i]);
                if (ImGui::Selectable(entity_names[i], is_selected))
                {
                    current_item = entity_names[i];
                }
                if (is_selected)
                {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }
        if (current_item != "(None)")
        {
            EntityHandle ent = getEntity(std::string(current_item));
            if (ent.valid())
            {
                if (ImGui::BeginTabBar("Components"))
                {
                    if (ent.has<Drawable>())
                    {
                        if (ImGui::BeginTabItem("Drawable"))
                        {
                            ent.get<Drawable>()->drawGUI();
                            ImGui::EndTabItem();
                        }
                    }
                    if (ent.has<Transform>())
                    {
                        if (ImGui::BeginTabItem("Transform"))
                        {
                            ent.get<Transform>()->drawGUI();
                            ImGui::EndTabItem();
                        }
                    }
                    if (ent.has<Material>())
                    {
                        if (ImGui::BeginTabItem("Material"))
                        {
                            ent.get<Material>()->drawGUI();
                            ImGui::EndTabItem();
                        }
                    }
                    if (ent.has<ForwardMaterial>())
                    {
                        if (ImGui::BeginTabItem("ForwardMaterial"))
                        {
                            ent.get<ForwardMaterial>()->drawGUI();
                            ImGui::EndTabItem();
                        }
                    }
                    if (ent.has<Camera>())
                    {
                        if (ImGui::BeginTabItem("Camera"))
                        {
                            ent.get<Camera>()->drawGUI();
                            ImGui::EndTabItem();
                        }
                    }
                    if (ent.has<DirectionalLight>())
                    {
                        if (ImGui::BeginTabItem("DirectionalLight"))
                        {
                            ent.get<DirectionalLight>()->drawGUI();
                            ImGui::EndTabItem();
                        }
                    }
                    if (ent.has<PointLight>())
                    {
                        if (ImGui::BeginTabItem("PointLight"))
                        {
                            ent.get<PointLight>()->drawGUI();
                            ImGui::EndTabItem();
                        }
                    }
                    ImGui::EndTabBar();
                }
            }
        }
    }
    ImGui::End();
}

void RCubeViewer::onResize(int w, int h)
{
    camera_.get<rcube::Camera>()->viewport_size = glm::ivec2(w, h);
}

void RCubeViewer::beforeTerminate()
{
    world_.cleanup();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

glm::vec2 RCubeViewer::screenToNDC(int xpos, int ypos)
{
    const float x = static_cast<float>(xpos);
    const float y = static_cast<float>(ypos);
    const glm::vec2 size(this->size());
    float ndc_x = (2.0f * xpos) / (float)size[0] - 1.0f;
    float ndc_y = 1.0f - (2.0f * ypos) / (float)size[1];
    return glm::vec2(ndc_x, ndc_y);
}

void RCubeViewer::updateImageBasedLighting()
{
    auto skybox = camera_.get<Camera>()->skybox;
    if (skybox != nullptr)
    {
        IBLDiffuse diff;
        std::shared_ptr<TextureCubemap> irradiance_map = diff.irradiance(skybox);
        IBLSpecularSplitSum spec;
        std::shared_ptr<TextureCubemap> prefilter_map = spec.prefilter(skybox);
        std::shared_ptr<Texture2D> brdf_lut = spec.integrateBRDF();
        camera_.get<Camera>()->irradiance = irradiance_map;
        camera_.get<Camera>()->prefilter = prefilter_map;
        camera_.get<Camera>()->brdfLUT = brdf_lut;
    }
}

///////////////////////////////////////////////////////////////////////////////

EntityHandle RCubeViewer::createSurface()
{
    auto ent = world_.createEntity();
    ent.add<Drawable>(Drawable());
    if (world_.getSystem("ForwardRenderSystem") != nullptr)
    {
        ent.add<ForwardMaterial>();
    }
    else if (world_.getSystem("DeferredRenderSystem") != nullptr)
    {
        ent.add<Material>();
    }
    ent.add<Transform>(Transform());
    return ent;
}

EntityHandle RCubeViewer::createCamera()
{
    auto ent = world_.createEntity();
    ent.add<Camera>(Camera());
    ent.add<Transform>(Transform());
    ent.add<CameraController>();
    return ent;
}

EntityHandle RCubeViewer::createGroundPlane()
{
    std::shared_ptr<Mesh> mesh = Mesh::create(plane(20, 20, 100, 100, Orientation::PositiveY));
    mesh->uploadToGPU();
    ground_ = createSurface();
    Material *mat = ground_.get<Material>();
    mat->albedo = glm::vec3(1);
    mat->roughness = 0.5f;
    mat->albedo_texture = Texture2D::create(1024, 1024, 1, TextureInternalFormat::sRGBA8);
    mat->albedo_texture->setData(checkerboard(1024, 1024, 32, 32, glm::vec3(1.f), glm::vec3(0.1f)));
    ground_.get<Transform>()->translate(glm::vec3(0, -1, 0));
    ground_.get<Drawable>()->mesh = mesh;
    ground_.add(Name("Ground"));
    return ground_;
}

EntityHandle RCubeViewer::createGroundGrid()
{
    std::shared_ptr<Mesh> mesh = Mesh::create(grid(20, 20, 100, 100, glm::vec3(0, 0, 1), glm::vec3(0, 1, 0), glm::vec3(0, 0, 0)));
    mesh->uploadToGPU();
    ground_ = createSurface();
    ForwardMaterial *mat = ground_.get<ForwardMaterial>();
    auto shader = std::make_shared<UnlitMaterial>();
    shader->use_vertex_colors = true;
    ground_.get<ForwardMaterial>()->shader = shader;
    ground_.get<Transform>()->translate(glm::vec3(0, -1, 0));
    ground_.get<Drawable>()->mesh = mesh;
    ground_.add(Name("Ground"));
    return ground_;
}

EntityHandle RCubeViewer::createDirLight()
{
    EntityHandle ent = world_.createEntity();
    ent.add(DirectionalLight());
    return ent;
}


AABB RCubeViewer::worldBoundingBox()
{
    EntityHandleIterator iter = world_.entities();
    AABB world_bbox;
    while (iter.hasNext())
    {
        EntityHandle ent = iter.next();
        if (ent.entity == ground_.entity)
        {
            continue;
        }
        if (ent.has<Drawable>() && ent.has<Transform>())
        {
            Drawable *dr = ent.get<Drawable>();
            if (!dr->visible)
            {
                continue;
            }
            Transform *tr = ent.get<Transform>();
            const glm::mat4 model2world = tr->worldTransform();
            auto positions = dr->mesh->attribute("positions")->ptrVec3();
            size_t num_vertices = dr->mesh->attribute("positions")->count();
            for (size_t i = 0; i < num_vertices; ++i)
            {
                glm::vec3 world_pos = glm::vec3(model2world * glm::vec4(positions[i], 1.f));
                world_bbox.expandBy(world_pos);
            }
        }
    }
    return world_bbox;
}

void RCubeViewer::fitCameraExtents()
{
    Camera *cam = camera().get<Camera>();
    Transform *cam_tr = camera().get<Transform>();
    AABB world_bbox = worldBoundingBox();
    glm::vec3 center = (world_bbox.min() + world_bbox.max()) / 2.f;
    float bounding_size = world_bbox.size()[glm::length_t(world_bbox.longestAxis())];

    // Don't try to fit if the bounding box is empty
    if (bounding_size < 1e-5)
    {
        return;
    }
    float tmp = 0.5f * cam->fov;
    float aspect = float(cam->viewport_size[0]) / float(cam->viewport_size[1]);
    if (aspect < 1.0)
    {
        tmp = std::atan(aspect * std::tan(tmp));
    }
    float distance = (bounding_size * 0.5f) / std::tan(tmp);
    glm::vec3 forward = glm::normalize(cam->target - cam_tr->worldPosition());
    cam->target = center;
    cam_tr->setPosition(center - forward * distance);
}

} // namespace viewer
} // namespace rcube