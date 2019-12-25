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
#include "RCube/Core/Graphics/TexGen/CheckerBoard.h"
#include "RCube/Controller/OrbitController.h"
#include "RCube/Core/Graphics/Materials/PhysicallyBasedMaterial.h"
#include "RCube/Core/Graphics/MeshGen/Plane.h"
#include "RCube/Core/Graphics/MeshGen/Box.h"
#include "RCube/Core/Graphics/TexGen/CheckerBoard.h"

class App : public rcube::Window
{
    rcube::Scene mScene;
    rcube::EntityHandle mGroundPlane;
    rcube::EntityHandle mCamera;
    rcube::EntityHandle mSphere;
    rcube::EntityHandle mSphereTex;
    rcube::EntityHandle mBox;
    rcube::EntityHandle mPlane;
    rcube::OrbitController mCtrl;

public:
    App() : rcube::Window("Hello sphere")
    {
        using namespace rcube;

        // Create ground plane
        std::shared_ptr<Mesh> gridMesh = Mesh::create();
        gridMesh->data = rcube::grid(2, 2, 10, 10,
            glm::vec3(1.0, 0.0, 0.0),
            glm::vec3(0.0, 1.0, 0.0),
            glm::vec3(0.0, 0.0, 0.0));
        gridMesh->uploadToGPU();
        std::shared_ptr<FlatMaterial> flat = std::make_shared<FlatMaterial>();
        mGroundPlane = mScene.createDrawable();
        mGroundPlane.get<Drawable>()->mesh = gridMesh;
        mGroundPlane.get<Drawable>()->material = flat;

        // Create a sphere with texture        
        {
            std::shared_ptr<Mesh> sphereMesh = Mesh::create();
            std::shared_ptr<BlinnPhongMaterial> blinnPhong =
                std::make_shared<BlinnPhongMaterial>(glm::vec3(1.0, 0.7, 0.8));
            blinnPhong->show_wireframe = false;
            blinnPhong->diffuse_texture = Texture2D::create(256, 256, 1);

            Image checker = checkerboard(256, 256, 32, 32, glm::vec3(1, 1, 1), glm::vec3(0, 1, 0));
            blinnPhong->diffuse_texture->setData(checker);
            blinnPhong->use_diffuse_texture = true;
            sphereMesh->data = cubeSphere(0.5, 10);
            sphereMesh->uploadToGPU();
            mSphereTex = mScene.createDrawable();
            mSphereTex.get<Drawable>()->mesh = sphereMesh;
            mSphereTex.get<Drawable>()->material = blinnPhong;
        }


         // Create a sphere
        {
            std::shared_ptr<Mesh> sphereMesh = Mesh::create();
            sphereMesh->data = icoSphere(0.5, 3);
            std::shared_ptr<BlinnPhongMaterial> blinnPhong =
                std::make_shared<BlinnPhongMaterial>(glm::vec3(1.0, 0.7, 0.8));
            blinnPhong->show_wireframe = true;
            blinnPhong->diffuse_texture = Texture2D::create(256, 256, 1);           
            sphereMesh->uploadToGPU();
            std::shared_ptr<PhysicallyBasedMaterial> pbm =
                std::make_shared<PhysicallyBasedMaterial>();
            pbm->roughness = 0.2;
            mSphere = mScene.createDrawable();
            mSphere.get<Drawable>()->mesh = sphereMesh;
            mSphere.get<Drawable>()->material = pbm;
            mSphere.get<Transform>()->translate(glm::vec3(1, 1, -1));
        }


        //create a plane
        {
            std::shared_ptr<Mesh> planeMesh = Mesh::create();
            planeMesh->data = plane(2, 2, 10, 10, rcube::Orientation::PositiveY);
            planeMesh->uploadToGPU();
            std::shared_ptr<BlinnPhongMaterial> blinnPhong =
                std::make_shared<BlinnPhongMaterial>(glm::vec3(1.0, 0.7, 0.8));
            blinnPhong->show_wireframe = true;
            mPlane = mScene.createDrawable();
            mPlane.get<Drawable>()->mesh = planeMesh;
            mPlane.get<Drawable>()->material = blinnPhong;
        }


        //create box with texture
        std::shared_ptr<Mesh> boxMesh = Mesh::create();
        boxMesh->data = box(1, 1, 1, 1, 1, 1);
        boxMesh->uploadToGPU();
        std::shared_ptr<BlinnPhongMaterial> tex_blinnPhone = std::make_shared<BlinnPhongMaterial>();
        tex_blinnPhone->diffuse_texture = Texture2D::create(128, 128, 1, rcube::TextureInternalFormat::RGBA8);
        tex_blinnPhone->diffuse_texture->setData(checkerboard(128, 128, 8, 8, glm::vec3(0, 0, 0), glm::vec3(1, 1, 1)));
        tex_blinnPhone->use_diffuse_texture = true;
        std::cout << tex_blinnPhone->diffuse_texture->valid() << std::endl;
        mBox = mScene.createDrawable();
        mBox.get<Drawable>()->mesh = boxMesh;
        mBox.get<Drawable>()->material = tex_blinnPhone;
        mBox.get<Transform>()->translate(glm::vec3(1, 1, 1));

        // Create a camera
        mCamera = mScene.createCamera();
        mCamera.get<Transform>()->setPosition(glm::vec3(0, 0, 1));
        mCamera.get<Camera>()->fov = glm::radians(30.0);
        mCamera.get<Camera>()->near_plane = 0.01f;
        mCamera.get<Camera>()->background_color = glm::vec4(0.3, 0.3, 0.3, 1.0);
        EntityHandle light1 = mScene.createPointLight();
        light1.get<Transform>()->setPosition(glm::vec3(1, 1, 0));

        EntityHandle light2 = mScene.createPointLight();
        light2.get<Transform>()->setParent(mCamera.get<Transform>());

        mCtrl.resize(1280, 720);
        mCtrl.setEntity(mCamera);

        //init scene
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
