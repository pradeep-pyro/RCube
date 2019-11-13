#include "RCube/RCube.h"
#include "RCube/Scene.h"
#include "RCube/Window.h"
#include "RCube/Core/Graphics/MeshGen/Sphere.h"
#include "RCube/Core/Graphics/MeshGen/Grid.h"
#include "RCube/Core/Graphics/OpenGL/CheckGLError.h"
#include "RCube/Components/Drawable.h"
#include "RCube/Components/Camera.h"
#include "RCube/Core/Graphics/Materials/BlinnPhongMaterial.h"
#include "RCube/Core/Graphics/Materials/FlatMaterial.h"
#include "RCube/Controller/OrbitController.h"


class App : public rcube::Window
{
    rcube::Scene mScene;
    rcube::EntityHandle mGroundPlane;
    rcube::EntityHandle mCamera;
    rcube::OrbitController mCtrl;

public:
    App() : rcube::Window("Hello sphere")
    {
        using namespace rcube;
        EntityHandle sphere = mScene.createDrawable();

        // Create gound plane
        std::shared_ptr<Mesh> gridMesh = Mesh::create();
        gridMesh->data = rcube::grid(2, 2, 10, 10, glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 0.0));
        gridMesh->uploadToGPU();
        std::shared_ptr<FlatMaterial> flat = std::make_shared<FlatMaterial>();
        mGroundPlane = mScene.createDrawable();
        mGroundPlane.get<Drawable>()->mesh = gridMesh;
        mGroundPlane.get<Drawable>()->material = flat;

        // Create a sphere
        std::shared_ptr<Mesh> sphereMesh = Mesh::create();
        std::shared_ptr<BlinnPhongMaterial> blinnPhong = std::make_shared<BlinnPhongMaterial>(glm::vec3(1.0, 0.7, 0.8));
        blinnPhong->show_wireframe = true;
        sphereMesh->data = icoSphere(0.5, 3);
        sphereMesh->uploadToGPU();
        sphere.get<Drawable>()->mesh = sphereMesh;
        sphere.get<Drawable>()->material = blinnPhong;

        // Create a camera
        mCamera = mScene.createCamera();
        mCamera.get<Transform>()->setPosition(glm::vec3(0, 0, 1));
        mCamera.get<Camera>()->fov = glm::radians(30.0);
        mCamera.get<Camera>()->near_plane = 0.01f;
        mCamera.get<Camera>()->background_color = glm::vec4(0.3, 0.3, 0.3, 1.0);
        EntityHandle light = mScene.createPointLight();
        light.get<Transform>()->setPosition(glm::vec3(1, 1, 0));
        mCtrl.resize(1280, 720);
        mCtrl.setEntity(mCamera);
        mScene.initialize();
    }

    virtual void draw() override
    {
        mScene.update();
    }

    virtual void onResize(int w, int h) override
    {
        mCtrl.resize(w, h);
        mCamera.get<rcube::Camera>()->viewport_size = glm::ivec2(w, h);
    }

    virtual void beforeTerminate() override
    {
        mScene.cleanup();
    }

    virtual void onKeyPress(int key, int mods)
    {
        if (key == GLFW_KEY_PAGE_UP)
        {
            mCtrl.zoom(-1.0);
        }
        if (key == GLFW_KEY_PAGE_DOWN)
        {
            mCtrl.zoom(1.0);
        }
    }

    virtual void onMousePress(int key, int mods)
    {
        glm::dvec2 pos = getMousePosition();
        if (key == GLFW_MOUSE_BUTTON_MIDDLE)
        {
            mCtrl.startPanning(pos.x, pos.y);
        }
        else if (key == GLFW_MOUSE_BUTTON_RIGHT)
        {
            mCtrl.startOrbiting(pos.x, pos.y);
        }
    }
    virtual void onMouseRelease(int key, int mods)
    {
        glm::dvec2 pos = getMousePosition();
        if (key == GLFW_MOUSE_BUTTON_MIDDLE)
        {
            mCtrl.stopPanning(pos.x, pos.y);
        }
        else if (key == GLFW_MOUSE_BUTTON_RIGHT)
        {
            mCtrl.stopOrbiting(pos.x, pos.y);
        }
    }
    virtual void onMouseMove(double xpos, double ypos)
    {
        glm::dvec2 pos = getMousePosition();
        mCtrl.orbit(pos.x, pos.y);
        mCtrl.pan(pos.x, pos.y);
    }
};

int main()
{
    App viewer;
    viewer.execute();
    return 0;
}
