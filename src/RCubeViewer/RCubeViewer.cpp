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

namespace rcube
{
namespace viewer
{

RCubeViewer::RCubeViewer(RCubeViewerProps props)
    : Window(props.title), transform_widgets_(props.transform_widgets)
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
    // ImGuizmo
    ImGuizmo::BeginFrame();
    ImGuizmo::Enable(true);
    ImGuiIO &io = ImGui::GetIO();
    ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);

    // Create GUI
    drawGUI();
    customGUI(*this);

    // Render everything in the scene
    world_.update();
    ImGui::Render();
}

void RCubeViewer::drawMainMenuBarGUI()
{
    ImGui::BeginMainMenuBar();
    if (ImGui::BeginMenu("File"))
    {
        if (ImGui::MenuItem("Screenshot"))
        {
            needs_screenshot_ = true;
        }
        if (ImGui::MenuItem("Exit"))
        {
            this->shouldClose(true);
        }
        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Camera"))
    {
        Camera *cam = camera_.get<Camera>();
        CameraController *cam_ctrl = camera_.get<CameraController>();
        Transform *cam_tr = camera_.get<Transform>();
        cam->drawGUI();
        ImGui::Separator();
        cam_ctrl->drawGUI();
        ImGui::Separator();
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
        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Transform"))
    {
        if (ImGui::MenuItem("Translate", "T",
                            transform_widgets_.operation == TransformOperation::Translate))
        {
            transform_widgets_.operation = TransformOperation::Translate;
        }
        if (ImGui::MenuItem("Rotate", "R",
                            transform_widgets_.operation == TransformOperation::Rotate))
        {
            transform_widgets_.operation = TransformOperation::Rotate;
        }
        if (ImGui::MenuItem("Scale", "S",
                            transform_widgets_.operation == TransformOperation::Scale))
        {
            transform_widgets_.operation = TransformOperation::Scale;
        }
        if (ImGui::MenuItem("None", "Esc",
                            transform_widgets_.operation == TransformOperation::None))
        {
            transform_widgets_.operation = TransformOperation::None;
        }
        ImGui::Separator();
        if (ImGui::MenuItem("Local space", nullptr,
                            transform_widgets_.space == ImGuizmo::MODE::LOCAL))
        {
            transform_widgets_.space = ImGuizmo::MODE::LOCAL;
        }
        if (ImGui::MenuItem("World space", nullptr,
                            transform_widgets_.space == ImGuizmo::MODE::WORLD))
        {
            transform_widgets_.space = ImGuizmo::MODE::WORLD;
        }
        ImGui::Separator();
        static float snap_pos = 0.f;
        if (ImGui::SliderFloat("Snap tr.", &snap_pos, 0.f, 1.f))
        {
            transform_widgets_.snap_translation = glm::vec3(snap_pos);
        }
        static int snap_angle = 0;
        if (ImGui::SliderInt("Snap ang. (deg.)", &snap_angle, 0, 90))
        {
            transform_widgets_.snap_angle_degrees = static_cast<float>(snap_angle);
        }
        ImGui::SliderFloat("Snap sc.", &transform_widgets_.snap_scale, 0.1f, 1.f);
        ImGui::EndMenu();
    }
    ImGui::SameLine(ImGui::GetWindowWidth() - 200);
    ImGui::PushItemWidth(-500);
    ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate,
                ImGui::GetIO().Framerate);
    ImGui::PopItemWidth();
    ImGui::EndMainMenuBar();
}

void RCubeViewer::drawEntityInspectorGUI()
{
    ImGui::Begin("Entity Inspector");

    Camera *cam = camera_.get<Camera>();
    Transform *cam_tr = camera_.get<Transform>();

    ImGui::Text("Entities");
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
            if (!ent.has<Name>())
            {
                continue;
            }
            entity_names.push_back(ent.get<Name>()->name.c_str());
        }

        static const char *current_item = entity_names[0];
        ImGui::PushItemWidth(-1);
        if (ImGui::ListBoxHeader(""))
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
            ImGui::ListBoxFooter();
        }
        ImGui::PopItemWidth();
        if (current_item != "(None)")
        {
            EntityHandle ent = getEntity(std::string(current_item));
            if (ent.valid())
            {
                Transform *tr = ent.get<Transform>();
                if (tr != nullptr)
                {
                    if (show_transform_widgets_)
                    {
                        tr->drawTransformWidget(
                            cam->worldToView(), cam->viewToProjection(),
                            transform_widgets_.operation,
                            transform_widgets_.operation == TransformOperation::Scale
                                ? ImGuizmo::MODE::LOCAL
                                : transform_widgets_.space,
                            transform_widgets_.snap_translation,
                            transform_widgets_.snap_angle_degrees, transform_widgets_.snap_scale);
                    }
                }
                ImGui::Separator();
                ImGui::Text("Components");
                ImGui::PushID(current_item);
                // if (ImGui::BeginTabBar("Components"))
                {
                    if (ent.has<Drawable>())
                    {
                        // if (ImGui::BeginTabItem("Drawable"))
                        if (ImGui::CollapsingHeader("Drawable", ImGuiTreeNodeFlags_DefaultOpen))
                        {
                            ent.get<Drawable>()->drawGUI();
                            // ImGui::EndTabItem();
                        }
                    }
                    if (tr != nullptr)
                    {
                        // if (ImGui::BeginTabItem("Transform"))
                        if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
                        {
                            tr->drawGUI();
                            // ImGui::EndTabItem();
                        }
                    }
                    if (ent.has<Material>())
                    {
                        // if (ImGui::BeginTabItem("Material"))
                        if (ImGui::CollapsingHeader("Material", ImGuiTreeNodeFlags_DefaultOpen))
                        {
                            ent.get<Material>()->drawGUI();
                            // ImGui::EndTabItem();
                        }
                    }
                    if (ent.has<ForwardMaterial>())
                    {
                        // if (ImGui::BeginTabItem("ForwardMaterial"))
                        if (ImGui::CollapsingHeader("ForwardMaterial",
                                                    ImGuiTreeNodeFlags_DefaultOpen))
                        {
                            ent.get<ForwardMaterial>()->drawGUI();
                            // ImGui::EndTabItem();
                        }
                    }
                    if (ent.has<Camera>())
                    {
                        // if (ImGui::BeginTabItem("Camera"))
                        if (ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_DefaultOpen))
                        {
                            ent.get<Camera>()->drawGUI();
                            // ImGui::EndTabItem();
                        }
                    }
                    if (ent.has<DirectionalLight>())
                    {
                        // if (ImGui::BeginTabItem("DirectionalLight"))
                        if (ImGui::CollapsingHeader("DirectionalLight",
                                                    ImGuiTreeNodeFlags_DefaultOpen))
                        {
                            ent.get<DirectionalLight>()->drawGUI();
                            // ImGui::EndTabItem();
                        }
                    }
                    if (ent.has<PointLight>())
                    {
                        // if (ImGui::BeginTabItem("PointLight"))
                        if (ImGui::CollapsingHeader("PointLight", ImGuiTreeNodeFlags_DefaultOpen))
                        {
                            ent.get<PointLight>()->drawGUI();
                            // ImGui::EndTabItem();
                        }
                    }
                    // ImGui::EndTabBar();
                }
                ImGui::PopID();
            }
        }
    }
    ImGui::End();
}

void RCubeViewer::drawGUI()
{
    ImGui::DockSpaceOverViewport(nullptr, ImGuiDockNodeFlags_PassthruCentralNode |
                                              ImGuiDockNodeFlags_NoDockingInCentralNode);

    drawMainMenuBarGUI();

    drawEntityInspectorGUI();
}

void RCubeViewer::onResize(int w, int h)
{
    camera_.get<rcube::Camera>()->viewport_size = glm::ivec2(w, h);
}

void RCubeViewer::beforeTerminate()
{
    world_.cleanup();
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
    ent.add<Drawable>();
    if (world_.getSystem("ForwardRenderSystem") != nullptr)
    {
        ent.add<ForwardMaterial>();
    }
    else if (world_.getSystem("DeferredRenderSystem") != nullptr)
    {
        ent.add<Material>();
    }
    ent.add<Transform>();
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
    // ground_.get<Transform>()->translate(glm::vec3(0, -1, 0));
    ground_.get<Drawable>()->mesh = mesh;
    ground_.add(Name("Ground"));
    return ground_;
}

EntityHandle RCubeViewer::createGroundGrid()
{
    std::shared_ptr<Mesh> mesh = Mesh::create(
        grid(20, 20, 100, 100, glm::vec3(0, 0, 1), glm::vec3(0, 1, 0), glm::vec3(0, 0, 0)));
    mesh->uploadToGPU();
    ground_ = createSurface();
    ForwardMaterial *mat = ground_.get<ForwardMaterial>();
    auto shader = std::make_shared<UnlitMaterial>();
    shader->use_vertex_colors = true;
    ground_.get<ForwardMaterial>()->shader = shader;
    // ground_.get<Transform>()->translate(glm::vec3(0, -1, 0));
    ground_.get<Drawable>()->mesh = mesh;
    ground_.add(Name("Ground"));
    return ground_;
}

EntityHandle RCubeViewer::createDirLight()
{
    EntityHandle ent = world_.createEntity();
    ent.add<DirectionalLight>();
    ent.get<DirectionalLight>()->setDirection(glm::vec3(0, -1, 0));
    return ent;
}

void RCubeViewer::onKeyPress(int key, int scancode)
{
    if (key == GLFW_KEY_ESCAPE)
    {
        transform_widgets_.operation = TransformOperation::None;
    }
    else if (key == GLFW_KEY_T)
    {
        transform_widgets_.operation = TransformOperation::Translate;
    }
    else if (key == GLFW_KEY_R)
    {
        transform_widgets_.operation = TransformOperation::Rotate;
    }
    else if (key == GLFW_KEY_S)
    {
        transform_widgets_.operation = TransformOperation::Scale;
    }
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