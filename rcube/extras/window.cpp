#include "window.h"
#include "../rcube.h"
#include <iostream>

namespace rcube {

///////////////////////////////////////////////////////////////////////////////
// Low-level GLFW callbacks

static void callbackOnError(int, const char* err_str) {
    std::cerr << "GLFW Error: " << err_str << std::endl;
}

static void callbackOnResize(GLFWwindow *window, int w, int h) {
    static_cast<Window *>(glfwGetWindowUserPointer(window))->onResize(w, h);
}

static void callbackKeyboard(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {
        static_cast<Window *>(glfwGetWindowUserPointer(window))->onKeyPress(key, mods);
    }
    else if (action == GLFW_RELEASE) {
        static_cast<Window *>(glfwGetWindowUserPointer(window))->onKeyRelease(key, mods);
    }
    else if (action == GLFW_REPEAT) {
        static_cast<Window *>(glfwGetWindowUserPointer(window))->onKeyRepeat(key, mods);
    }
}

static void callbackMouse(GLFWwindow* window, int button, int action, int mods) {
    if (action == GLFW_PRESS) {
        static_cast<Window *>(glfwGetWindowUserPointer(window))->onMousePress(button, mods);
    }
    else if (action == GLFW_RELEASE) {
        static_cast<Window *>(glfwGetWindowUserPointer(window))->onMouseRelease(button, mods);
    }
}

static void callbackMouseMove(GLFWwindow* window, double xpos, double ypos) {
    static_cast<Window *>(glfwGetWindowUserPointer(window))->onMouseMove(xpos, ypos);
}

///////////////////////////////////////////////////////////////////////////////

Window::Window(const std::string &title, glm::ivec2 size) {
    glfwSetErrorCallback(callbackOnError);
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
    window_ = glfwCreateWindow(size.x, size.y, title.c_str(),
                               nullptr, nullptr);
    glfwMakeContextCurrent(window_);
    glfwSwapInterval(1); // Enable vsync
    rcube::initGL();

    glfwSetWindowUserPointer(window_, this);
    glfwSetFramebufferSizeCallback(window_, callbackOnResize);
    glfwSetKeyCallback(window_, callbackKeyboard);
    glfwSetMouseButtonCallback(window_, callbackMouse);
    glfwSetCursorPosCallback(window_, callbackMouseMove);
}

void Window::initialize() {
}

void Window::draw() {
}

void Window::onResize(int width, int height) {
    std::cout << "(w, h) : " << width << ", " << height << std::endl;
}

void Window::onKeyPress(int key, int mods) {
}

void Window::onKeyRelease(int key, int mods) {
}

void Window::onKeyRepeat(int key, int mods) {
}

void Window::onMousePress(int button, int mods) {
}

void Window::onMouseRelease(int button, int mods) {
}

void Window::onMouseMove(double xpos, double ypos) {
}

glm::dvec2 Window::getMousePosition() const {
    glm::dvec2 xy;
    glfwGetCursorPos(window_, &xy[0], &xy[1]);
    return xy;
}

void Window::execute() {
    initialize(); // User should override this method
    while (!glfwWindowShouldClose(window_)) {
        draw(); // User should override this method
        glfwPollEvents();
        glfwSwapBuffers(window_);
    }
    beforeTerminate(); // User should override this method
    glfwDestroyWindow(window_);
    glfwTerminate();
}

void Window::beforeTerminate() {
}

bool Window::fullscreen() const {
    return fullscreen_;
}

void Window::setFullscreen(bool flag) {
    if (fullscreen_ && flag) {
        return;
    }

    if (fullscreen_) {
        glfwGetWindowPos(window_, &wndpos_.x, &wndpos_.y);
        glfwGetWindowSize(window_, &wndsz_.x, &wndsz_.y);
        GLFWmonitor* primary = glfwGetPrimaryMonitor();
        const GLFWvidmode* mode = glfwGetVideoMode(primary);
        //glfwSetWindowMonitor(window_, primary, 0, 0, mode->width, mode->height, 0);
        fullscreen_ = true;
    }
    else {
        // restore last window size and position
        //glfwSetWindowMonitor(window_, nullptr, wndpos_.x, wndpos_.y, wndsz_.x, wndsz_.y, 0);
        fullscreen_ = false;
    }
}

void Window::shouldClose(bool flag) {
    glfwSetWindowShouldClose(window_, flag);
}

glm::ivec2 Window::size() const {
    glm::ivec2 sz;
    glfwGetWindowSize(window_, &sz.x, &sz.y);
    return sz;
}

} // namespace rcube
