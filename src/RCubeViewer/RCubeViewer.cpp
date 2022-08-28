#include "RCubeViewer/RCubeViewer.h"
#include "RCube/Core/Arch/World.h"
#include "RCube/Core/Graphics/ImageBasedLighting/IBLDiffuse.h"
#include "RCube/Core/Graphics/ImageBasedLighting/IBLSpecularSplitSum.h"
#include "RCube/Core/Graphics/MeshGen/Plane.h"
#include "RCube/Core/Graphics/TexGen/CheckerBoard.h"
#include "RCube/Core/Graphics/TexGen/Gradient.h"
#include "RCube/Systems/DeferredRenderSystem.h"
#include "RCubeViewer/Components/Name.h"
#include "glm/gtx/euler_angles.hpp"
#include "glm/gtx/string_cast.hpp"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <chrono>
#include <filesystem>
#include <iomanip>
#include <sstream>

namespace rcube
{
namespace viewer
{

void initImGUI(GLFWwindow *window)
{
    IMGUI_CHECKVERSION();

    ImGui::CreateContext();

    // Set up ImGUI glfw bindings
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    const char *glsl_version = "#version 420";
    ImGui_ImplOpenGL3_Init(glsl_version);

    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    ImFontConfig config;
    config.OversampleH = 5;
    config.OversampleV = 5;

    ImGui::StyleColorsClassic();
}

RCubeViewer::RCubeViewer(RCubeViewerProps props)
    : Window(props.title), transform_widgets_(props.transform_widgets),
      time_per_frame_(1.0 / static_cast<double>(props.frames_per_second))
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

    // Assign camera to controller
    ctrl_.setCamera(camera_.get<Camera>(), camera_.get<Transform>());

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

void RCubeViewer::selectEntity(const std::string &name)
{
    selected_entity_ = name;
}

void RCubeViewer::getEntityAtCoord(double xpos, double ypos, EntityHandle &entity,
                                   size_t &primitiveId)
{
    auto render_base_system = world_.getSystem("ForwardRenderSystem");
    if (render_base_system == nullptr)
    {
        return;
    }
    ForwardRenderSystem *render_system = dynamic_cast<ForwardRenderSystem *>(render_base_system);
    if (render_system == nullptr)
    {
        return;
    }
    // Get the texture holding the picking information
    std::shared_ptr<Texture2D> objPrimID = render_system->objPrimIDTexture();
    if (objPrimID != nullptr)
    {
        int w, h;
        glfwGetWindowSize(window_, &w, &h);

        // Initialize double-buffered PBOs if not already available
        if (pbos_.first() == nullptr)
        {
            pbos_.first() = PixelPackBuffer::create(size_t(w) * size_t(h), GL_STREAM_READ);
        }
        if (pbos_.second() == nullptr)
        {
            pbos_.second() = PixelPackBuffer::create(size_t(w) * size_t(h), GL_STREAM_READ);
        }
        // Transform mouse coordinate to texture image space
        xpos /= w;
        ypos /= h;
        ypos = 1.0 - ypos;
        xpos *= objPrimID->width();
        ypos *= objPrimID->height();
        // Return if mouse is outside window
        if (xpos < 0 || ypos < 0 || xpos >= objPrimID->width() || ypos >= objPrimID->height())
        {
            return;
        }
        // Copy a single pixel from the texture asynchronously
        pbos_.first()->use();
        objPrimID->getSubImage(int(xpos), int(ypos), 1, 1, (uint32_t *)NULL, 2);
        pbos_.second()->use();
        GLuint *ptr = (GLuint *)pbos_.second()->map();
        if (ptr != nullptr)
        {
            entity.world = &world_;
            entity.entity = ptr[0]; // Entity ID
            primitiveId = ptr[1];   // Primitive ID
            pbos_.second()->unmap();
        }
        pbos_.second()->done();
        // Swap the buffers
        pbos_.increment();
    }
}

const std::string &RCubeViewer::selectedEntity() const
{
    return selected_entity_;
}

void RCubeViewer::initialize()
{
    initImGUI(window_);
    // Update the PBR image-based lighting maps for all object's materials
    updateImageBasedLighting();
}

void RCubeViewer::draw()
{
    // Render everything in the scene
    double now = time();
    double delta_time = now - last_time_;
    {
        // Initialize ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        // ImGuizmo
        ImGuizmo::BeginFrame();
        ImGuizmo::Enable(true);
        ImGuiIO &io = ImGui::GetIO();
        ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);
        io.DeltaTime = static_cast<float>(delta_time);

        // Create GUI
        drawGUI();

        world_.update();
        last_time_ = now;
        current_fps_ = 1.0 / delta_time;
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }
}

void RCubeViewer::drawMainMenuBarGUI()
{
    ImGui::BeginMainMenuBar();

    if (ImGui::BeginMenu("File"))
    {
        if (ImGui::MenuItem("Screenshot"))
        {
            auto render_system = world_.getSystem<ForwardRenderSystem>();
            if (render_system != nullptr)
            {
                auto im = render_system->screenshot();
                // Get timestamp for filename
                auto time = std::time(nullptr);
                std::stringstream ss;
                ss << std::put_time(std::localtime(&time), "%F_%T");
                screenshot_filename_ = ss.str();
                std::replace(screenshot_filename_.begin(), screenshot_filename_.end(), ':', '-');
                screenshot_filename_ += ".bmp";
                screenshot_filename_ = std::filesystem::absolute(screenshot_filename_).string();
                im.saveBMP(screenshot_filename_, true);
                time_point_ = std::chrono::high_resolution_clock::now();
            }
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
        Transform *cam_tr = camera_.get<Transform>();
        cam->drawGUI();
        ImGui::Separator();
        ctrl_.drawGUI();
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
    if (std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::high_resolution_clock::now() - time_point_)
            .count() <= 3000)
    {
        ImGui::SameLine(0.f, 10.f);
        ImGui::Text(("Saved to: " + screenshot_filename_).c_str());
    }
    ImGui::SameLine(ImGui::GetWindowWidth() - 200);
    ImGui::PushItemWidth(-500);
    /*ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate,
                ImGui::GetIO().Framerate);*/
    ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / current_fps_, current_fps_);
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

        ImGui::PushItemWidth(-1);
        if (ImGui::ListBoxHeader(""))
        {
            for (int i = 0; i < entity_names.size(); ++i)
            {
                bool is_selected = (selected_entity_ == entity_names[i]);
                if (ImGui::Selectable(entity_names[i], is_selected))
                {
                    selected_entity_ = entity_names[i];
                }
                if (is_selected)
                {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::ListBoxFooter();
        }
        ImGui::PopItemWidth();
        if (selected_entity_ != "(None)")
        {
            EntityHandle ent = getEntity(selected_entity_);
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
                ImGui::PushID(selected_entity_.c_str());
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
    // Destroy ImGui
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

void RCubeViewer::clearSelection()
{
    selected_entity_ = "(None)";
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

void RCubeViewer::onMousePress(int button, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_RIGHT)
    {
        ctrl_.startRotation(getMousePosition());
    }
    if (button == GLFW_MOUSE_BUTTON_MIDDLE)
    {
        ctrl_.startPanning(getMousePosition());
    }
}

void RCubeViewer::onMouseRelease(int button, int mods)
{
    ctrl_.stopPanning();
    ctrl_.stopRotation();
}

void RCubeViewer::onMouseMove(double xpos, double ypos)
{
    ctrl_.rotate(xpos, ypos);
    ctrl_.pan(xpos, ypos);
}

void RCubeViewer::onScroll(double xoffset, double yoffset)
{
    ctrl_.zoom(yoffset);
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
    AABB world_bbox = worldBoundingBox();
    cam->fitToExtents(world_bbox);
    ctrl_.zoom_speed = world_bbox.diagonal() * 0.1f;
}

} // namespace viewer
} // namespace rcube