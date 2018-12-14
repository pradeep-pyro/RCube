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
#include "rcube/meshes/superformula.h"
#include "rcube/materials/FlatMaterial.h"
#include "rcube/materials/BlinnPhongMaterial.h"
#include "rcube/systems/CameraSystem.h"
#include "rcube/controller/OrbitController.h"
#include "rcube/effects/GrayscaleEffect.h"
#include "rcube/effects/BlurEffect.h"
#include "rcube/render/checkglerror.h"

rcube::OrbitController ctrl;

static void onError(int, const char* err_str) {
    std::cout << "GLFW Error: " << err_str << std::endl;
}

static void controllerCallback(GLFWwindow *window) {
    double xpos, ypos;
    rcube::CameraController::InputState state;
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
}


static void onResize(GLFWwindow * /*window*/, int w, int h) {
    ctrl.resize(w, h);
    ctrl.camera()->resize(w, h);
};

EntityHandle setupCamera(rcube::Scene &scene) {
    double s = glfwGetTime();
    EntityHandle cam = scene.createCamera();
    double e = glfwGetTime();
    cout << "createCamera took : " << e - s << "s" << endl;
    cam.get<rcube::Transform>()->setPosition(glm::vec3(0, 2, 4));

    std::vector<Image> ims {Image::fromFile("/home/pradeep/Downloads/Yokohama/posx.jpg", 3),
                            Image::fromFile("/home/pradeep/Downloads/Yokohama/negx.jpg", 3),
                            Image::fromFile("/home/pradeep/Downloads/Yokohama/posy.jpg", 3),
                            Image::fromFile("/home/pradeep/Downloads/Yokohama/negy.jpg", 3),
                            Image::fromFile("/home/pradeep/Downloads/Yokohama/posz.jpg", 3),
                            Image::fromFile("/home/pradeep/Downloads/Yokohama/negz.jpg", 3)};
    for (int i = 0; i < 6; ++i) {
        cam.get<rcube::Camera>()->skybox->setData(i, ims[i]);
    }
    cam.get<rcube::Camera>()->use_skybox = true;
    //cam.get<rcube::Camera>()->postprocess.push_back(std::make_shared<BlurEffect>());
    //cam.get<rcube::Camera>()->postprocess.push_back(std::make_shared<GrayscaleEffect>());

    ctrl.setEntity(cam);
    return cam;
}

int main(int, char**) {
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
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        throw std::runtime_error("Failed to initialize OpenGL context");
    }
    glfwSetFramebufferSizeCallback(window, onResize);

    double start = glfwGetTime();
    rcube::Scene scene;
    double end = glfwGetTime();

    start = glfwGetTime();
    EntityHandle cam = setupCamera(scene);
    end = glfwGetTime();
    cout << "setupCamera took " << end - start << "s" << endl;

    start = glfwGetTime();
    EntityHandle gridobj = scene.createDrawable();
    gridobj.get<rcube::Drawable>()->mesh = Mesh::create();
    //gridobj.get<rcube::Drawable>()->mesh->setMeshData(grid(3, 3, 10, 10, glm::vec3(1, 0, 0), glm::vec3(0, 1, 0), glm::vec3(0.4)));
    gridobj.get<rcube::Drawable>()->mesh->setMeshData(grid(3, 3, 10, 10, glm::vec3(1, 0, 0), glm::vec3(0, 1, 0), glm::vec3(0.4)));
    gridobj.get<rcube::Drawable>()->material = std::make_shared<FlatMaterial>();

    start = glfwGetTime();
    EntityHandle circ = scene.createDrawable();
    circ.get<rcube::Drawable>()->mesh = Mesh::create();
    circ.get<rcube::Drawable>()->mesh->setMeshData(cylinder(1, 1.5, 3, 30, 10));
    cout << "createDrawable cylinder took " << end - start << "s" << endl;

    auto phong = std::make_shared<BlinnPhongMaterial>();
    phong->diffuse_color = glm::vec3(0, 0, 1);
    phong->environment_map = cam.get<rcube::Camera>()->skybox;
    phong->use_environment_map = true;
    //phong->show_wireframe = true;
    circ.get<rcube::Drawable>()->material = phong;

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
