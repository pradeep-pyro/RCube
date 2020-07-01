#include "RCubeViewer/RCubeViewer.h"
#include "RCube/Core/Arch/World.h"
#include "RCube/Core/Graphics/ImageBasedLighting/IBLDiffuse.h"
#include "RCube/Core/Graphics/ImageBasedLighting/IBLSpecularSplitSum.h"
#include "RCube/Core/Graphics/Materials/PhysicallyBasedMaterial.h"
#include "RCube/Core/Graphics/Effects/GammaCorrectionEffect.h"
#include "RCube/Core/Graphics/TexGen/Gradient.h"
#include "RCubeViewer/Components/CameraController.h"
#include "RCubeViewer/Components/Name.h"
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
    // Make a default skybox
    camera_.get<Camera>()->skybox = TextureCubemap::create(256, 256);
    glm::vec3 color_top =
        glm::pow(glm::vec3(123.f / 255.f, 154.f / 255.f, 203.f / 255.f), glm::vec3(2.2f));
    glm::vec3 color_bot =
        glm::pow(glm::vec3(100.f / 255.f, 93 / 255.f, 86.f / 255.f), glm::vec3(2.2f));
    Image front_back = gradientV(256, 256, color_top, color_bot);
    Image top = gradientV(256, 256, color_top, color_top);
    Image bottom = gradientV(256, 256, color_bot, color_bot);
    camera_.get<Camera>()->skybox->setData(TextureCubemap::PositiveY, top);
    camera_.get<Camera>()->skybox->setData(TextureCubemap::NegativeY, bottom);
    camera_.get<Camera>()->skybox->setData(TextureCubemap::PositiveX, front_back);
    camera_.get<Camera>()->skybox->setData(TextureCubemap::NegativeX, front_back);
    camera_.get<Camera>()->skybox->setData(TextureCubemap::NegativeZ, front_back);
    camera_.get<Camera>()->skybox->setData(TextureCubemap::PositiveZ, front_back);
    camera_.get<Camera>()->use_skybox = true;
    // Add gamma correction
    camera_.get<Camera>()->postprocess.push_back(makeGammaCorrectionEffect());
    // Put a directional light on the camera
    camera_.add<DirectionalLight>();
    // Create a ground plane
    ground_ = createGroundPlane();
    ground_.get<Drawable>()->visible = props.ground_plane;

    world_.initialize();

    IMGUI_CHECKVERSION();
    initImGUI(window_);
}

EntityHandle RCubeViewer::addSurface(const std::string name, const TriangleMeshData &data)
{
    EntityHandle ent = createSurface();
    ent.add<Name>(name);

    std::shared_ptr<Mesh> mesh = Mesh::create(data);
    std::shared_ptr<Material> mat = std::make_shared<PhysicallyBasedMaterial>();
    mesh->uploadToGPU();
    ent.get<Drawable>()->mesh = mesh;
    ent.get<Drawable>()->material = mat;
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
    ImGui::Render();
    world_.update();

    // Draw GUI
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
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

        camera_.get<Camera>()->drawGUI();
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
                    if (ent.has<Camera>())
                    {
                        if (ImGui::BeginTabItem("Camera"))
                        {
                            ent.get<Camera>()->drawGUI();
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
        std::shared_ptr<TextureCubemap> irradiance_map;
        std::shared_ptr<TextureCubemap> prefilter_map;
        std::shared_ptr<Texture2D> brdf_lut;
        {
            IBLDiffuse diff;
            irradiance_map = diff.irradiance(skybox);
            IBLSpecularSplitSum spec;
            prefilter_map = spec.prefilter(skybox);
            brdf_lut = spec.integrateBRDF();
        }

        for (auto ent_it = world_.entities(); ent_it.hasNext();)
        {
            auto ent = ent_it.next();
            if (ent.has<Drawable>())
            {
                auto mat = ent.get<Drawable>()->material;
                auto pbrmat = std::dynamic_pointer_cast<PhysicallyBasedMaterial>(mat);
                if (pbrmat != nullptr)
                {
                    pbrmat->setIBLMaps(irradiance_map, prefilter_map, brdf_lut);
                }
            }
        }
    }
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
    /*std::shared_ptr<Mesh> gridMesh =
        Mesh::create(grid(20, 20, 100, 100, glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0),
                          glm::vec3(0.0, 0.0, 0.0)));*/
    std::shared_ptr<Mesh> mesh =
        Mesh::create(grid(20, 20, 100, 100, glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0),
                          glm::vec3(0.0, 0.0, 0.0)));
    mesh->uploadToGPU();
    auto mat = std::make_shared<FlatMaterial>();
    ground_ = createSurface();
    ground_.get<Drawable>()->mesh = mesh;
    ground_.get<Drawable>()->material = mat;
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