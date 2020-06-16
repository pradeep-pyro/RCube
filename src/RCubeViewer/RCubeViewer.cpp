#include "RCubeViewer/RCubeViewer.h"
#include "RCube/Core/Arch/World.h"
#include "RCubeViewer/Components/Name.h"
#include "RCubeViewer/Components/CameraController.h"
#include "RCubeViewer/Systems/CameraControllerSystem.h"
#include "RCubeViewer/Systems/PickSystem.h"
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

    ImGui::StyleColorsLight();
}

RCubeViewer::RCubeViewer(RCubeViewerProps props) : Window(props.title)
{
    world_.addSystem(std::make_unique<TransformSystem>());
    world_.addSystem(std::make_unique<CameraSystem>());
    world_.addSystem(std::make_unique<RenderSystem>(props.resolution, props.MSAA));
    world_.addSystem(std::make_unique<CameraControllerSystem>());
    world_.addSystem(std::make_unique<PickSystem>());

    // Create a default camera
    camera_ = createCamera();
    camera_.get<Transform>()->lookAt(glm::vec3(0.f, 0.f, 1.5f), glm::vec3(0.f, 0.f, 0.f),
                                     YAXIS_POSITIVE);
    camera_.get<Camera>()->fov = props.camera_fov;
    camera_.get<Camera>()->background_color = props.background_color;
    camera_.get<Camera>()->orthographic = props.camera_orthographic;
    // Put a directional light on the camera
    camera_.add<DirectionalLight>();
    // Create a ground plane
    ground_ = createGroundPlane();
    ground_.get<Drawable>()->visible = props.ground_plane;

    world_.initialize();

    IMGUI_CHECKVERSION();
    initImGUI(window_);
}

EntityHandle RCubeViewer::addIcoSphereSurface(const std::string name, float radius,
                                              int numSubdivisions)
{
    EntityHandle ent = createSurface();
    ent.add<Name>(name);

    std::shared_ptr<Mesh> sphereMesh = Mesh::create(icoSphere(radius, numSubdivisions));
    std::shared_ptr<ShaderProgram> blinnPhong = makeBlinnPhongMaterial();
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

    std::shared_ptr<Mesh> sphereMesh = Mesh::create(cubeSphere(radius, numSegments));
    std::shared_ptr<ShaderProgram> blinnPhong = makeBlinnPhongMaterial();
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

    std::shared_ptr<Mesh> boxMesh =
        Mesh::create(box(width, height, depth, width_segments, height_segments, depth_segments));
    std::shared_ptr<ShaderProgram> blinnPhong = makeBlinnPhongMaterial();
    boxMesh->uploadToGPU();
    ent.get<Drawable>()->mesh = boxMesh;
    ent.get<Drawable>()->material = blinnPhong;
    return ent;
}

EntityHandle RCubeViewer::addSurface(const std::string name, const TriangleMeshData &data)
{
    EntityHandle ent = createSurface();
    ent.add<Name>(name);

    std::shared_ptr<Mesh> mesh = Mesh::create(data);
    std::shared_ptr<ShaderProgram> blinnPhong = makeBlinnPhongMaterial();
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
    customGUI(*this);

    // Render everything in the scene
    ImGui::Render();
    world_.update();

    // Draw GUI
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void drawGUIForScalarFieldComponent(EntityHandle ent)
{
    /*ScalarField *sf = ent.get<ScalarField>();
    ImGui::Checkbox("Show", &sf->show);
    ImGui::InputFloat("vmin", &sf->vmin);
    ImGui::InputFloat("vmax", &sf->vmax);
    const char *colormap_names[5] = {"Viridis", "Plasma", "Magma", "Inferno", "Jet"};
    int curr_colormap = sf->colormap;
    if (ImGui::Combo("Colormap", &curr_colormap, colormap_names, IM_ARRAYSIZE(colormap_names)))
    {
        sf->colormap = static_cast<ScalarField::Colormap>(curr_colormap);
    }*/
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
    static const char *current_attr = nullptr;
    if (ImGui::BeginCombo("Attribute", current_attr))
    {
        for (auto &kv : dr->mesh->attributes())
        {
            bool is_selected =
                (current_attr == kv.first.c_str());
            if (ImGui::Selectable(kv.first.c_str(), is_selected))
            {
                current_attr = kv.first.c_str();
            }
            if (is_selected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }
    if (current_attr != nullptr)
    {
        ImGui::LabelText("Count", std::to_string(dr->mesh->attribute(current_attr)->size()).c_str());
        ImGui::LabelText("Dimension", std::to_string(dr->mesh->attribute(current_attr)->dim()).c_str());
        ImGui::LabelText("Layout location",
                         std::to_string(dr->mesh->attribute(current_attr)->location()).c_str());
        bool checked = dr->mesh->attributeEnabled(current_attr);
        if (ImGui::Checkbox("Active", &checked))
        {
            checked ? dr->mesh->enableAttribute(current_attr)
                    : dr->mesh->disableAttribute(current_attr);
        }
    }
    ImGui::LabelText(
        "#Faces", std::to_string(dr->mesh->indices()->size() / dr->mesh->primitiveDim()).c_str());
    ImGui::Separator();

    // Material
    const auto &uniforms = dr->material->availableUniforms();
    for (const ShaderUniformDesc &uni : uniforms)
    {
        switch (uni.type)
        {
        case GLDataType::Bool:
        {
            bool uni_bool;
            dr->material->uniform(uni.name).get(uni_bool);
            if (ImGui::Checkbox(uni.name.c_str(), &uni_bool))
            {
                dr->material->uniform(uni.name).set(uni_bool);
            }
            break;
        }
        case GLDataType::Int:
        {
            int uni_int;
            dr->material->uniform(uni.name).get(uni_int);
            if (ImGui::InputInt(uni.name.c_str(), &uni_int))
            {
                dr->material->uniform(uni.name).set(uni_int);
            }
            break;
        }
        case GLDataType::Float:
        {
            float uni_float;
            dr->material->uniform(uni.name).get(uni_float);
            if (ImGui::InputFloat(uni.name.c_str(), &uni_float))
            {
                dr->material->uniform(uni.name).set(uni_float);
            }
            break;
        }
        case GLDataType::Vec2f:
        {
            glm::vec2 uni_vec2f;
            dr->material->uniform(uni.name).get(uni_vec2f);
            if (ImGui::InputFloat2(uni.name.c_str(), glm::value_ptr(uni_vec2f)))
            {
                dr->material->uniform(uni.name).set(uni_vec2f);
            }
            break;
        }
        case GLDataType::Vec3f:
        {
            glm::vec3 uni_vec3f;
            dr->material->uniform(uni.name).get(uni_vec3f);
            if (ImGui::InputFloat3(uni.name.c_str(), glm::value_ptr(uni_vec3f)))
            {
                dr->material->uniform(uni.name).set(uni_vec3f);
            }
            break;
        }
        case GLDataType::Color3f:
        {
            glm::vec3 uni_vec3f;
            dr->material->uniform(uni.name).get(uni_vec3f);
            if (ImGui::ColorEdit3(uni.name.c_str(), glm::value_ptr(uni_vec3f)))
            {
                dr->material->uniform(uni.name).set(uni_vec3f);
            }
            break;
        }
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

    ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate,
                ImGui::GetIO().Framerate);

    ///////////////////////////////////////////////////////////////////////////
    // Default camera
    if (ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_DefaultOpen))
    {
        Camera *cam = camera_.get<Camera>();
        Transform *cam_tr = camera_.get<Transform>();

        // Default camera viewpoints
        if (ImGui::Button("+X"))
        {
            cam->target = glm::vec3(0.f);
            cam_tr->lookAt(glm::vec3(+1.5f, 0, 0), glm::vec3(0.f), YAXIS_POSITIVE);
        }
        ImGui::SameLine();
        if (ImGui::Button("-X"))
        {
            cam->target = glm::vec3(0.f);
            cam_tr->lookAt(glm::vec3(-1.5f, 0, 0), glm::vec3(0.f), YAXIS_POSITIVE);
        }
        ImGui::SameLine();
        if (ImGui::Button("+Y"))
        {
            cam->target = glm::vec3(0.f);
            cam_tr->lookAt(glm::vec3(0, +1.5f, 0), glm::vec3(0.f), ZAXIS_POSITIVE);
        }
        ImGui::SameLine();
        if (ImGui::Button("-Y"))
        {
            cam->target = glm::vec3(0.f);
            cam_tr->lookAt(glm::vec3(0, -1.5f, 0), glm::vec3(0.f), ZAXIS_POSITIVE);
        }
        ImGui::SameLine();
        if (ImGui::Button("+Z"))
        {
            cam->target = glm::vec3(0.f);
            cam_tr->lookAt(glm::vec3(0, 0, +1.5f), glm::vec3(0.f), YAXIS_POSITIVE);
        }
        ImGui::SameLine();
        if (ImGui::Button("-Z"))
        {
            cam->target = glm::vec3(0.f);
            cam_tr->lookAt(glm::vec3(0, 0, -1.5f), glm::vec3(0.f), YAXIS_POSITIVE);
        }

        drawGUIForCameraComponent(camera_);
    }

    ///////////////////////////////////////////////////////////////////////////

    if (ImGui::CollapsingHeader("Objects", ImGuiTreeNodeFlags_DefaultOpen))
    {
        auto it = world_.entities();

        std::vector<const char *> entity_names;
        entity_names.reserve(world_.numEntities() + 1);
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
                    if (ent.has<Drawable>())
                    {
                        if (ImGui::BeginTabItem("Drawable"))
                        {
                            drawGUIForDrawableComponent(ent);
                            ImGui::EndTabItem();
                        }
                    }
                    if (ent.has<Transform>())
                    {
                        if (ImGui::BeginTabItem("Transform"))
                        {
                            drawGUIForTransformComponent(ent);
                            ImGui::EndTabItem();
                        }
                    }
                    if (ent.has<Camera>())
                    {
                        if (ImGui::BeginTabItem("Camera"))
                        {
                            drawGUIForCameraComponent(ent);
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

///////////////////////////////////////////////////////////////////////////////

EntityHandle RCubeViewer::createSurface()
{
    auto ent = world_.createEntity();
    ent.add<Drawable>(Drawable());
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
    std::shared_ptr<Mesh> gridMesh =
        Mesh::create(grid(20, 20, 100, 100, glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0),
                          glm::vec3(0.0, 0.0, 0.0)));
    gridMesh->uploadToGPU();
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