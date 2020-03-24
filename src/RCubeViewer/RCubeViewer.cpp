#include "RCubeViewer/RCubeViewer.h"
#include "RCube/Core/Arch/World.h"
#include "RCubeViewer/Components/Name.h"
#include "RCubeViewer/Components/ScalarField.h"
#include "glm/gtx/euler_angles.hpp"
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

    ImGui::StyleColorsLight();
}

RCubeViewer::RCubeViewer(RCubeViewerProps props) : Window(props.title)
{
    world_.addSystem(std::make_unique<TransformSystem>());
    world_.addSystem(std::make_unique<CameraSystem>());
    // auto rs = std::make_unique<RenderSystem>(props.resolution, props.MSAA);
    auto rs = std::make_unique<ViewerRenderSystem>(props.resolution, props.MSAA);
    world_.addSystem(std::move(rs));

    // Create a default camera
    camera_ = createCamera();
    camera_.get<Transform>()->setPosition(props.camera_position);
    camera_.get<Camera>()->fov = props.camera_fov;
    camera_.get<Camera>()->background_color = props.background_color;
    camera_.get<Camera>()->orthographic = props.camera_orthographic;
    // Put a directional light on the camera
    camera_.add<DirectionalLight>();

    // Create a ground plane
    ground_ = createGroundPlane();
    ground_.get<Drawable>()->visible = props.ground_plane;

    ctrl_.resize(static_cast<float>(props.resolution.x), static_cast<float>(props.resolution.y));
    ctrl_.setEntity(camera_);
    world_.initialize();

    IMGUI_CHECKVERSION();
    initImGUI(window_);
}

EntityHandle RCubeViewer::addIcoSphereSurface(const std::string name, float radius,
                                              int numSubdivisions)
{
    EntityHandle ent = createSurface();
    ent.add<Name>(name);

    std::shared_ptr<Mesh> sphereMesh = Mesh::create();
    std::shared_ptr<ShaderProgram> blinnPhong = makeBlinnPhongMaterial();
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
    std::shared_ptr<ShaderProgram> blinnPhong = makeBlinnPhongMaterial();
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
    std::shared_ptr<ShaderProgram> blinnPhong = makeBlinnPhongMaterial();
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
    std::shared_ptr<ShaderProgram> blinnPhong = makeBlinnPhongMaterial();
    mesh->data = data;
    mesh->uploadToGPU();
    ent.get<Drawable>()->mesh = mesh;
    ent.get<Drawable>()->material = blinnPhong;
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
    // Initialize ImGui
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // Create GUI
    drawGUI();

    // Render everything in the scene
    ImGui::Render();
    world_.update();

    // Draw GUI
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void drawGUIForScalarFieldComponent(EntityHandle ent)
{
    ScalarField *sf = ent.get<ScalarField>();
    ImGui::Checkbox("Show", &sf->show);
    ImGui::InputFloat("vmin", &sf->vmin);
    ImGui::InputFloat("vmax", &sf->vmax);
    const char *colormap_names[5] = {"Viridis", "Plasma", "Magma", "Inferno", "Jet"};
    int curr_colormap = sf->colormap;
    if (ImGui::Combo("Colormap", &curr_colormap, colormap_names, IM_ARRAYSIZE(colormap_names)))
    {
        sf->colormap = static_cast<ScalarField::Colormap>(curr_colormap);
    }
}

void drawGUIForTransformComponent(EntityHandle ent)
{
    // TODO: think of a way to handle transform hierarchy
    Transform *tr = ent.get<Transform>();
    static float xyz[3];
    const glm::vec3 &pos = tr->position();
    xyz[0] = pos[0];
    xyz[1] = pos[1];
    xyz[2] = pos[2];
    if (ImGui::InputFloat3("Position", xyz, 2))
    {
        tr->setPosition(glm::vec3(xyz[0], xyz[1], xyz[2]));
    }

    static glm::vec3 euler = glm::eulerAngles(tr->orientation());
    if (ImGui::SliderAngle("Orientation X", glm::value_ptr(euler)))
    {
        tr->setOrientation(glm::quat(euler));
    }
    if (ImGui::SliderAngle("Orientation Y", glm::value_ptr(euler) + 2))
    {
        tr->setOrientation(glm::quat(euler));
    }
    if (ImGui::SliderAngle("Orientation Z", glm::value_ptr(euler) + 1))
    {
        tr->setOrientation(glm::quat(euler));
    }

    static glm::vec3 scale = tr->scale();
    if (ImGui::InputFloat3("Scale", glm::value_ptr(scale), 2))
    {
        tr->setScale(scale);
    }
}

void drawGUIForDrawableComponent(EntityHandle ent)
{
    Drawable *dr = ent.get<Drawable>();

    // Visibility
    ImGui::Checkbox("Visible", &(dr->visible));
    ImGui::Separator();

    // Mesh
    ImGui::LabelText("#vertices", std::to_string(dr->mesh->data.vertices.size()).c_str());
    ImGui::LabelText("#faces", std::to_string(dr->mesh->data.indices.size()).c_str());
    ImGui::LabelText("Has texcoords",
                     (dr->mesh->data.vertices.size() == dr->mesh->data.texcoords.size()) ? "True"
                                                                                         : "False");
    const bool has_colors = dr->mesh->data.vertices.size() == dr->mesh->data.colors.size();
    ImGui::LabelText("Has colors", has_colors ? "True" : "False");
    ImGui::LabelText("Indexed", dr->mesh->data.indexed ? "True" : "False");
    ImGui::Separator();

    // Material
    const auto &uniforms = dr->material->availableUniforms();
    for (const ShaderUniformDesc &uni : uniforms)
    {
        switch (uni.type)
        {
        case GLDataType::Bool:
            bool uni_bool;
            dr->material->uniform(uni.name).get(uni_bool);
            if (ImGui::Checkbox(uni.name.c_str(), &uni_bool))
            {
                dr->material->uniform(uni.name).set(uni_bool);
            }
            break;
        case GLDataType::Int:
            int uni_int;
            dr->material->uniform(uni.name).get(uni_int);
            if (ImGui::InputInt(uni.name.c_str(), &uni_int))
            {
                dr->material->uniform(uni.name).set(uni_int);
            }
            break;
        case GLDataType::Float:
            float uni_float;
            dr->material->uniform(uni.name).get(uni_float);
            if (ImGui::InputFloat(uni.name.c_str(), &uni_float))
            {
                dr->material->uniform(uni.name).set(uni_float);
            }
            break;
        case GLDataType::Vec2f:
            glm::vec2 uni_vec2f;
            dr->material->uniform(uni.name).get(uni_vec2f);
            if (ImGui::InputFloat2(uni.name.c_str(), glm::value_ptr(uni_vec2f)))
            {
                dr->material->uniform(uni.name).set(uni_vec2f);
            }
            break;
        case GLDataType::Vec3f:
            glm::vec3 uni_vec3f;
            dr->material->uniform(uni.name).get(uni_vec3f);
            if (ImGui::InputFloat3(uni.name.c_str(), glm::value_ptr(uni_vec3f)))
            {
                dr->material->uniform(uni.name).set(uni_vec3f);
            }
            break;
        }
    }
}

void drawGUIForCameraComponent(EntityHandle ent)
{
    Camera *camera = ent.get<Camera>();

    ImGui::Checkbox("Orthographic", &camera->orthographic);
    if (camera->orthographic)
    {
        ImGui::InputFloat("Orthographic Width", &camera->orthographic_size);
    }
    else
    {
        ImGui::SliderAngle("FOV (deg.)", &camera->fov, 5.f, 89.f);
    }
    ImGui::InputFloat("Near Plane", &camera->near_plane);
    ImGui::InputFloat("Far Plane", &camera->far_plane);
    ImGui::ColorEdit4("Background Color", glm::value_ptr(camera->background_color));
    ImGui::Checkbox("Skybox", &camera->use_skybox);
}

void RCubeViewer::drawGUI()
{
    ImGui::Begin("RCubeViewer");
    ///////////////////////////////////////////////////////////////////////////
    // Default camera
    if (ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_DefaultOpen))
    {
        auto pos = camera_.get<Transform>()->position();
        static float xyz[3];
        xyz[0] = pos[0];
        xyz[1] = pos[1];
        xyz[2] = pos[2];
        if (ImGui::InputFloat3("Position", xyz, 2))
        {
            camera_.get<Transform>()->setPosition(glm::vec3(xyz[0], xyz[1], xyz[2]));
            xyz[0] = pos[0];
            xyz[1] = pos[1];
            xyz[2] = pos[2];
        }

        drawGUIForCameraComponent(camera_);
    }

    ///////////////////////////////////////////////////////////////////////////

    if (ImGui::CollapsingHeader("Objects", ImGuiTreeNodeFlags_DefaultOpen))
    {
        auto it = world_.entities();

        std::vector<const char *> entity_names;
        entity_names.reserve(world_.numEntities());
        entity_names.push_back("(None)");
        while (it.hasNext())
        {
            EntityHandle ent = it.next();
            if (ent.entity == ground_.entity || ent.entity == camera_.entity)
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
                    Drawable *dr = nullptr;
                    Transform *tr = nullptr;
                    Camera *cam = nullptr;
                    ScalarField *sf = nullptr;
                    try
                    {
                        dr = ent.get<Drawable>();
                    }
                    catch (const std::exception &)
                    {
                    }
                    try
                    {
                        tr = ent.get<Transform>();
                    }
                    catch (const std::exception &)
                    {
                    }
                    try
                    {
                        cam = ent.get<Camera>();
                    }
                    catch (const std::exception &)
                    {
                    }
                    try
                    {
                        sf = ent.get<ScalarField>();
                    }
                    catch (const std::exception &)
                    {
                    }

                    if (dr != nullptr)
                    {
                        if (ImGui::BeginTabItem("Drawable"))
                        {
                            drawGUIForDrawableComponent(ent);
                            ImGui::EndTabItem();
                        }
                    }
                    if (tr != nullptr)
                    {
                        if (ImGui::BeginTabItem("Transform"))
                        {
                            drawGUIForTransformComponent(ent);
                            ImGui::EndTabItem();
                        }
                    }
                    if (cam != nullptr)
                    {
                        if (ImGui::BeginTabItem("Camera"))
                        {
                            drawGUIForCameraComponent(ent);
                            ImGui::EndTabItem();
                        }
                    }
                    if (sf != nullptr)
                    {
                        if (ImGui::BeginTabItem("Scalar Field"))
                        {
                            drawGUIForScalarFieldComponent(ent);
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
    ctrl_.resize(static_cast<float>(w), static_cast<float>(h));
    camera_.get<rcube::Camera>()->viewport_size = glm::ivec2(w, h);
}

void RCubeViewer::onScroll(double xoffset, double yoffset)
{
    ctrl_.zoom(static_cast<float>(yoffset));
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

bool RCubeViewer::pick(int xpos, int ypos, EntityHandle &ent, size_t id)
{
    Camera *cam = camera_.get<Camera>();
    Transform *cam_tr = camera_.get<Transform>();
    const glm::vec2 ndc = screenToNDC(xpos, ypos);
    const glm::vec4 ray_clip(ndc.x, ndc.y, -1.0, 1.0);
    glm::vec4 ray_eye = glm::inverse(cam->viewToProjection()) * ray_clip;
    ray_eye.z = -1.f;
    ray_eye.w = 0.f;
    glm::vec3 ray_wor(glm::inverse(cam->worldToView()) * ray_eye);
    ray_wor = glm::normalize(ray_wor);

    // Closest hit info
    float min_dist = std::numeric_limits<float>::infinity();
    EntityHandle closest;
    size_t closest_id = 0;
    glm::vec3 closest_point;
    bool hit = false;
    for (auto iter = world_.entities(); iter.hasNext();)
    {
        EntityHandle e = iter.next();
        if (!e.has<Drawable>() || !e.has<Transform>())
        {
            continue;
        }
        Drawable *dr = e.get<Drawable>();
        if (!dr->visible)
        {
            continue;
        }
        Transform *tr = e.get<Transform>();
        const glm::mat4 model_inv = glm::inverse(tr->worldTransform());
        glm::vec3 ray_origin_model = glm::vec3(model_inv * glm::vec4(cam_tr->worldPosition(), 1.0));
        glm::vec3 ray_dir_model = glm::normalize(model_inv * glm::vec4(ray_wor, 0.0));
        Ray ray_model(ray_origin_model, ray_dir_model);
        glm::vec3 pt;
        size_t id;
        if (dr->mesh->rayIntersect(ray_model, pt, id))
        {
            hit = true;
            min_dist = std::min(glm::length(pt - ray_model.origin()), min_dist);
            closest_point = pt;
            closest_id = id;
            closest = e;
        }
    }
    if (hit)
    {
        ent = closest;
        id = closest_id;
        return true;
    }
    return false;
}

void RCubeViewer::onMousePress(int key, int mods)
{
    glm::dvec2 pos = getMousePosition();
    if (key == GLFW_MOUSE_BUTTON_MIDDLE)
    {
        ctrl_.startPanning(static_cast<int>(pos.x), static_cast<int>(pos.y));
    }
    else if (key == GLFW_MOUSE_BUTTON_RIGHT)
    {
        ctrl_.startOrbiting(static_cast<int>(pos.x), static_cast<int>(pos.y));
    }
    else if (key == GLFW_MOUSE_BUTTON_LEFT)
    {
        EntityHandle ent;
        size_t id = 0;
        pick(static_cast<int>(pos.x), static_cast<int>(pos.y), ent, id);
    }
}
void RCubeViewer::onMouseRelease(int key, int mods)
{
    glm::dvec2 pos = getMousePosition();
    if (key == GLFW_MOUSE_BUTTON_MIDDLE)
    {
        ctrl_.stopPanning(static_cast<int>(pos.x), static_cast<int>(pos.y));
    }
    else if (key == GLFW_MOUSE_BUTTON_RIGHT)
    {
        ctrl_.stopOrbiting(static_cast<int>(pos.x), static_cast<int>(pos.y));
    }
}
void RCubeViewer::onMouseMove(double xpos, double ypos)
{
    glm::dvec2 pos = getMousePosition();
    ctrl_.orbit(static_cast<int>(pos.x), static_cast<int>(pos.y));
    ctrl_.pan(static_cast<int>(pos.x), static_cast<int>(pos.y));
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
    gridMesh->data = rcube::grid(20, 20, 100, 100, glm::vec3(1.0, 0.0, 0.0),
                                 glm::vec3(0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 0.0));
    gridMesh->uploadToGPU();
    /*std::shared_ptr<FlatMaterial> flat = std::make_shared<FlatMaterial>();*/
    std::shared_ptr<ShaderProgram> flat = makeFlatMaterial();
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