#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"

#include <iostream>
#include <memory>
#include "glm/gtx/string_cast.hpp"
#include "rcube/components/Drawable.h"
#include "rcube/components/PointLight.h"
#include "rcube/Scene.h"
#include "rcube/meshes/plane.h"
#include "rcube/meshes/box.h"
#include "rcube/meshes/grid.h"
#include "rcube/meshes/cylinder.h"
#include "rcube/meshes/circle.h"
#include "rcube/meshes/supershape.h"
#include "rcube/materials/FlatMaterial.h"
#include "rcube/materials/BlinnPhongMaterial.h"
#include "rcube/controller/OrbitController.h"
#include "rcube/effects/GrayscaleEffect.h"
#include "rcube/effects/GammaCorrectionEffect.h"
#include "rcube/render/checkglerror.h"
#include "rcube/initgl.h"
#include "rcube/texgen/checkerboard.h"


rcube::OrbitController ctrl;
//rcube::PanZoomController ctrl;
rcube::CameraController::InputState state;

static void onError(int, const char* err_str) {
    std::cout << "GLFW Error: " << err_str << std::endl;
}

static void scrollCallback(GLFWwindow *window, double xoffset, double yoffset) {
    state.scroll_x = xoffset;
    state.scroll_y = yoffset;
    ctrl.update(state);
    state = rcube::CameraController::InputState();
}

static void controllerCallback(GLFWwindow *window) {
    double xpos, ypos;

    glfwGetCursorPos(window, &xpos, &ypos);
    state.x = int(xpos);
    state.y = int(ypos);
    state.alt = glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS;
    state.ctrl = glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS;
    state.shift = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS;
    state.mouse_left = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
    state.mouse_middle = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS;
    state.mouse_right = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;
    ctrl.update(state);
    state = rcube::CameraController::InputState();
}


static void onResize(GLFWwindow * /*window*/, int w, int h) {
    ctrl.resize(w, h);
    ctrl.camera()->resize(w, h);
};

EntityHandle setupCamera(rcube::Scene &scene) {
    double s = glfwGetTime();
    EntityHandle cam = scene.createCamera();
    cam.get<rcube::Camera>()->fov = glm::radians(30.f);
    //cam.get<rcube::Camera>()->orthographic = true;
    double e = glfwGetTime();
    cout << "createCamera took : " << e - s << "s" << endl;
    cam.get<rcube::Transform>()->setPosition(glm::vec3(0, 2, 4));

    std::vector<Image> ims {Image::fromFile("/home/pradeep/Downloads/Yokohama/posx.jpg", 3),
                            Image::fromFile("/home/pradeep/Downloads/Yokohama/negx.jpg", 3),
                            Image::fromFile("/home/pradeep/Downloads/Yokohama/posy.jpg", 3),
                            Image::fromFile("/home/pradeep/Downloads/Yokohama/negy.jpg", 3),
                            Image::fromFile("/home/pradeep/Downloads/Yokohama/posz.jpg", 3),
                            Image::fromFile("/home/pradeep/Downloads/Yokohama/negz.jpg", 3)};
    cam.get<rcube::Camera>()->skybox = TextureCubemap::create(ims[0].width(), ims[0].height());
    for (int i = 0; i < 6; ++i) {
        cam.get<rcube::Camera>()->skybox->setData(i, ims[i]);
    }
    cam.get<rcube::Camera>()->use_skybox = true;
    //cam.get<rcube::Camera>()->postprocess.push_back(std::make_shared<GrayscaleEffect>());
    cam.get<rcube::Camera>()->postprocess.push_back(std::make_shared<GammaCorrectionEffect>());

    ctrl.setEntity(cam);
    return cam;
}

int main(int, char**) {
    rcube::checkerboard(500, 500, 50, 50, glm::vec3(255, 0, 0), glm::vec3(0, 255, 0));
    glfwSetErrorCallback(onError);
    if (!glfwInit()) {
        throw std::runtime_error("Failed to initialize GLFW");
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    GLFWwindow* window = glfwCreateWindow(1280, 720, "RCube Demo", NULL, NULL);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    rcube::initGL();
    glfwSetFramebufferSizeCallback(window, onResize);
    glfwSetScrollCallback(window, scrollCallback);

    rcube::Scene scene;
    EntityHandle cam = setupCamera(scene);

    EntityHandle cube = scene.createDrawable();
    auto cube_drawable = cube.get<rcube::Drawable>();
    cube_drawable->mesh = Mesh::create();
    cube_drawable->mesh->data = box(2, 2, 2, 5, 5, 5);
    cube_drawable->mesh->uploadToGPU();
    auto phong = std::make_shared<BlinnPhongMaterial>();
    auto diff = Texture2D::create(500, 500, 1, TextureInternalFormat::RGBA8);
    auto spec = Texture2D::create(500, 500, 1, TextureInternalFormat::R8);
    diff->setData(Image::fromFile("/home/pradeep/diffuse.png", 3));
    //diff->setData(rcube::checkerboard(500, 500, 50, 50, glm::vec3(0), glm::vec3(255)));
    spec->setData(Image::fromFile("/home/pradeep/specular.png", 1));
    phong->diffuse_texture = diff;
    phong->specular_texture = spec;
    phong->shininess = 64.f;
    phong->use_diffuse_texture = true;
    phong->use_specular_texture = true;
    phong->show_wireframe = true;
    phong->wireframe_color = glm::vec3(0,1,1);
    cube_drawable->material = phong;
    phong.reset(); // don't extend phong's life
    diff.reset();
    spec.reset();

    EntityHandle light = scene.createPointLight();
    light.get<rcube::PointLight>()->setRadius(10.f);
    light.get<rcube::Transform>()->setPosition(glm::vec3(0));
    light.get<rcube::Transform>()->setParent(cam.get<rcube::Transform>());

    double previous_time = 0;
    size_t frame_count = 0;

    scene.initialize();

    while (!glfwWindowShouldClose(window)) {
        // Measure speed...
        double current_time = glfwGetTime();
        ++frame_count;
        // ...if a second has passed.
        if (current_time - previous_time >= 1.0) {
            cout << "FPS: " << frame_count << endl;
            frame_count = 0;
            previous_time = current_time;
        }

        // Update and draw the scene
        scene.update();

        glfwPollEvents();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);

        controllerCallback(window);


        glfwSwapBuffers(window);
    }

    // Cleanup
    scene.cleanup();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
